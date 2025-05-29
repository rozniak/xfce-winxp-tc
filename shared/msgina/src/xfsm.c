#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/xfsm.h"

//
// FORWARD DECLARATIONS
//
static void wintc_gina_sm_xfce_dispose(
    GObject* object
);

static gboolean wintc_gina_sm_xfce_call_can(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name
);
static gboolean wintc_gina_sm_xfce_call_do(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name,
    GError**         error
);

static void on_bus_xfsm_name_appeared(
    GDBusConnection* connection,
    const gchar*     name,
    const gchar*     name_owner,
    gpointer         user_data
);
static void on_bus_xfsm_name_vanished(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCGinaSmXfce
{
    GObject __parent__;

    // State stuff
    //
    guint       id_bus_watcher;
    GDBusProxy* proxy;
} WinTCGinaSmXfce;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCGinaSmXfce,
    wintc_gina_sm_xfce,
    G_TYPE_OBJECT
)

static void wintc_gina_sm_xfce_class_init(
    WinTCGinaSmXfceClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_gina_sm_xfce_dispose;
}

static void wintc_gina_sm_xfce_init(
    WinTCGinaSmXfce* self
)
{
    self->id_bus_watcher =
        g_bus_watch_name(
            G_BUS_TYPE_SESSION,
            "org.xfce.SessionManager",
            G_BUS_NAME_WATCHER_FLAGS_NONE,
            on_bus_xfsm_name_appeared,
            on_bus_xfsm_name_vanished,
            self,
            NULL
        );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_sm_xfce_dispose(
    GObject* object
)
{
    WinTCGinaSmXfce* sm_xfce = WINTC_GINA_SM_XFCE(object);

    g_bus_unwatch_name(sm_xfce->id_bus_watcher);

    (G_OBJECT_CLASS(wintc_gina_sm_xfce_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXfce* wintc_gina_sm_xfce_new(void)
{
    return WINTC_GINA_SM_XFCE(
        g_object_new(
            WINTC_TYPE_GINA_SM_XFCE,
            NULL
        )
    );
}

gboolean wintc_gina_sm_xfce_can_restart(
    WinTCGinaSmXfce* sm_xfce
)
{
    return wintc_gina_sm_xfce_call_can(
        sm_xfce,
        "CanRestart"
    );
}

gboolean wintc_gina_sm_xfce_can_shut_down(
    WinTCGinaSmXfce* sm_xfce
)
{
    return wintc_gina_sm_xfce_call_can(
        sm_xfce,
        "CanShutdown"
    );
}

gboolean wintc_gina_sm_xfce_can_sleep(
    WinTCGinaSmXfce* sm_xfce
)
{
    return wintc_gina_sm_xfce_call_can(
        sm_xfce,
        "CanHybridSleep"
    );
}

gboolean wintc_gina_sm_xfce_log_off(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
)
{
    return wintc_gina_sm_xfce_call_do(
        sm_xfce,
        "Logout",
        error
    );
}

gboolean wintc_gina_sm_xfce_restart(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            sm_xfce,
            "CanRestart"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        sm_xfce,
        "Restart",
        error
    );
}

gboolean wintc_gina_sm_xfce_shut_down(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            sm_xfce,
            "CanShutdown"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        sm_xfce,
        "Shutdown",
        error
    );
}

gboolean wintc_gina_sm_xfce_sleep(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            sm_xfce,
            "CanHybridSleep"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        sm_xfce,
        "HyrbidSleep",
        error
    );
}

gboolean wintc_gina_sm_xfce_switch_user(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
)
{
    return wintc_gina_sm_xfce_call_do(
        sm_xfce,
        "SwitchUser",
        error
    );
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_gina_sm_xfce_call_can(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name
)
{
    if (!(sm_xfce->proxy))
    {
        WINTC_LOG_USER_DEBUG(
            "gina: attempted to call %s but no connection to xfsm",
            method_name
        );

        return FALSE;
    }

    GVariant* result_v =
        g_dbus_proxy_call_sync(
            sm_xfce->proxy,
            method_name,
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            NULL
        );

    gboolean result = g_variant_get_boolean(result_v);

    g_variant_unref(result_v);

    return result;
}

static gboolean wintc_gina_sm_xfce_call_do(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name,
    GError**         error
)
{
    //
    // Assume the caller actually bothered to call the CanX method before this
    // one
    //
    GVariant* params_v = NULL;

    if (g_strcmp0(method_name, "Logout") == 0)
    {
        params_v = g_variant_new("(bb)", FALSE, FALSE);
    }
    else if (
        g_strcmp0(method_name, "Restart") == 0 ||
        g_strcmp0(method_name, "Shutdown") == 0
    )
    {
        params_v = g_variant_new("(b)", FALSE);
    }

    g_dbus_proxy_call_sync(
        sm_xfce->proxy,
        method_name,
        params_v,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    return !!error;
}

//
// CALLBACKS
//
static void on_bus_xfsm_name_appeared(
    GDBusConnection* connection,
    const gchar*     name,
    WINTC_UNUSED(const gchar* name_owner),
    gpointer         user_data
)
{
    WinTCGinaSmXfce* sm_xfce = WINTC_GINA_SM_XFCE(user_data);

    GError* error = NULL;

    sm_xfce->proxy =
        g_dbus_proxy_new_sync(
            connection,
            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
            NULL,
            name,
            "/org/xfce/SessionManager",
            "org.xfce.Session.Manager",
            NULL,
            &error
        );

    if (!(sm_xfce->proxy))
    {
        wintc_log_error_and_clear(&error);
        return;
    }
}

static void on_bus_xfsm_name_vanished(
    WINTC_UNUSED(GDBusConnection* connection),
    WINTC_UNUSED(const gchar*     name),
    gpointer user_data
)
{
    WinTCGinaSmXfce* sm_xfce = WINTC_GINA_SM_XFCE(user_data);

    g_clear_object(&(sm_xfce->proxy));
}
