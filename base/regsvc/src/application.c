#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/registry.h>

#include "application.h"
#include "backend.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCRegSvcApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCRegSvcApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_regsvc_application_activate(
    GApplication* application
);
static gint wintc_regsvc_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
);
static gint wintc_regsvc_application_handle_local_options(
    GApplication* application,
    GVariantDict* options
);
static void wintc_regsvc_application_shutdown(
    GApplication* application
);

static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

static gboolean on_handle_create_key(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    gpointer               user_data
);
static gboolean on_handle_get_key_value(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    gpointer               user_data
);
static gboolean on_handle_set_key_value(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    const gchar*           value_name,
    GVariant*              value_data,
    gboolean               silent,
    gpointer               user_data
);

//
// STATIC DATA
//
static const GOptionEntry S_OPTIONS[] = {
    {
        "quit",
        'q',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_NONE,
        NULL,
        "Quit a running Windows registry instance.",
        NULL
    },
    { 0 }
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCRegSvcApplication,
    wintc_regsvc_application,
    GTK_TYPE_APPLICATION
)

static void wintc_regsvc_application_class_init(
    WinTCRegSvcApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate =
        wintc_regsvc_application_activate;
    application_class->command_line =
        wintc_regsvc_application_command_line;
    application_class->handle_local_options =
        wintc_regsvc_application_handle_local_options;
    application_class->shutdown =
        wintc_regsvc_application_shutdown;
}

static void wintc_regsvc_application_init(
    WinTCRegSvcApplication* self
)
{
    g_application_add_main_option_entries(
        G_APPLICATION(self),
        S_OPTIONS
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_regsvc_application_activate(
    GApplication* application
)
{
    static gboolean started = FALSE;

    if (started)
    {
        return;
    }

    started = TRUE;

    // Init
    //
    if (!backend_init())
    {
        g_critical("%s", "Failed to initialize backend.");
        g_application_quit(application);
        return;
    }

    WINTC_LOG_DEBUG("regsvc: hosting name %s", "uk.oddmatics.wintc.registry");

    g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "uk.oddmatics.wintc.registry",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        NULL,
        on_name_acquired,
        NULL,
        application,
        NULL
    );

    // Hold open since we have no toplevels
    //
    g_application_hold(application);
}

static gint wintc_regsvc_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
)
{
    GVariantDict* options =
        g_application_command_line_get_options_dict(command_line);

    // Just check for --quit
    //
    if (g_variant_dict_contains(options, "quit"))
    {
        WINTC_LOG_DEBUG("%s", "regsvc: --quit recieved, exiting...");

        g_application_release(application);
        return 0;
    }

    g_application_activate(application);

    return 0;
}

static gint wintc_regsvc_application_handle_local_options(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GVariantDict* options)
)
{
    // Stub
    return -1;
}

static void wintc_regsvc_application_shutdown(
    GApplication* application
)
{
    backend_close();

    (G_APPLICATION_CLASS(wintc_regsvc_application_parent_class))
        ->shutdown(application);
}

//
// PUBLIC FUNCTIONS
//
WinTCRegSvcApplication* wintc_regsvc_application_new(void)
{
    return WINTC_REGSVC_APPLICATION(
        g_object_new(
            wintc_regsvc_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.regsvc",
            "flags",          G_APPLICATION_HANDLES_COMMAND_LINE,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_name_acquired(
    GDBusConnection* connection,
    WINTC_UNUSED(const gchar* name),
    gpointer         user_data
)
{
    GError*            error    = NULL;
    ZWinRegistryGDBUS* reg_dbus = zwin_registry_gdbus_skeleton_new();

    g_signal_connect(
        reg_dbus,
        "handle-create-key",
        G_CALLBACK(on_handle_create_key),
        NULL
    );
    g_signal_connect(
        reg_dbus,
        "handle-get-key-value",
        G_CALLBACK(on_handle_get_key_value),
        NULL
    );
    g_signal_connect(
        reg_dbus,
        "handle-set-key-value",
        G_CALLBACK(on_handle_set_key_value),
        NULL
    );

    if (
        !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(reg_dbus),
            connection,
            "/uk/oddmatics/wintc/registry/GDBUS",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        g_application_quit(G_APPLICATION(user_data));
    }
}

static gboolean on_handle_create_key(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("regsvc: received create key request for %s", key_path);

    // Very simply just try creating the key
    //
    gboolean success = backend_create_key(key_path);

    zwin_registry_gdbus_complete_create_key(
        reg_dbus,
        invocation,
        success ? 1 : 0
    );

    return TRUE;
}

static gboolean on_handle_get_key_value(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG(
        "regsvc: received get key value for %s->%s",
        key_path,
        value_name
    );

    // Wrap up in a variant
    //
    gboolean  success          = FALSE;
    gint      value_data_int   = 0;
    gint64    value_data_int64 = 0;
    gchar*    value_data_str   = NULL;
    GVariant* variant_ret      = NULL;

    switch (value_type)
    {
        case WINTC_REG_DWORD:
            success =
                backend_get_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_DWORD,
                    &value_data_int
                );

            if (success)
            {
                variant_ret =
                    g_variant_new(
                        "v",
                        g_variant_new_int32(value_data_int)
                    );
            }

            break;

        case WINTC_REG_QWORD:
            success =
                backend_get_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_QWORD,
                    &value_data_int64
                );

            if (success)
            {
                variant_ret =
                    g_variant_new(
                        "v",
                        g_variant_new_int64(value_data_int64)
                    );
            }

            break;

        case WINTC_REG_SZ:
            success =
                backend_get_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_SZ,
                    &value_data_str
                );

            if (success)
            {
                variant_ret =
                    g_variant_new(
                        "v",
                        g_variant_new_string(value_data_str)
                    );

                g_free(value_data_str);
            }

            break;

        default:
            g_critical("regsvc: unknown type %d", value_type);
            success = FALSE;
            break;
    }

    if (!success)
    {
        // Cannot use maybe-type variants because they're not supported by
        // DBus/GDBus, so just respond with 0 - libwintc-registry will discard
        // the variant regardless if the result code is 0 so we could respond
        // with any variant here and be fine
        //
        variant_ret = g_variant_new("v", g_variant_new_int32(0));
    }

    zwin_registry_gdbus_complete_get_key_value(
        reg_dbus,
        invocation,
        variant_ret,
        success ? 1 : 0
    );

    return TRUE;
}

static gboolean on_handle_set_key_value(
    ZWinRegistryGDBUS*     reg_dbus,
    GDBusMethodInvocation* invocation,
    const gchar*           key_path,
    const gchar*           value_name,
    GVariant*              value_data,
    gboolean               silent,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG(
        "regsvc: received set key value for %s->%s",
        key_path,
        value_name
    );

    // The variant incoming determines the type etc.
    //
    GVariant*              inner      = g_variant_get_variant(value_data);
    gboolean               success    = FALSE;
    WinTCRegistryValueType value_type = wintc_registry_get_type_for_variant(
                                            inner
                                        );

    gint value_data_int     = 0;
    gint64 value_data_int64 = 0;
    gchar* value_data_str   = NULL;

    switch (value_type)
    {
        case WINTC_REG_DWORD:
            value_data_int = g_variant_get_int32(inner);

            success =
                backend_set_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_DWORD,
                    &value_data_int
                );

            break;

        case WINTC_REG_QWORD:
            value_data_int64 = g_variant_get_int64(inner);

            success =
                backend_set_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_QWORD,
                    &value_data_int64
                );

            break;

        case WINTC_REG_SZ:
            value_data_str = g_variant_dup_string(inner, NULL);

            success =
                backend_set_key_value(
                    key_path,
                    value_name,
                    WINTC_REG_SZ,
                    &value_data_str
                );

            g_free(value_data_str);
            break;

        default:
            g_critical("%s", "regsvc: set key unknown variant type");
            break;
    }

    zwin_registry_gdbus_complete_set_key_value(
        reg_dbus,
        invocation,
        success ? 1 : 0
    );

    if (success & !silent)
    {
        g_signal_emit_by_name(
            reg_dbus,
            "key-value-changed",
            key_path,
            value_name,
            value_data,
            NULL
        );
    }

    g_variant_unref(inner);

    return TRUE;
}
