#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "../snw-dbus.h"
#include "icon.h"
#include "sni.h"

//
// FORWARD DECLARATIONS
//
static void wintc_notification_sni_constructed(
    GObject* object
);

static gboolean on_handle_register_status_notifier_item(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
);
static gboolean on_handle_register_status_notifier_host(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
);

static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);
static void on_name_lost(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationSni
{
    WinTCShextUIController __parent__;

    ZWinKdeStatusNotifierWatcher* dbus_snw;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationSni,
    wintc_notification_sni,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_notification_sni_class_init(
    WinTCNotificationSniClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_sni_constructed;
}

static void wintc_notification_sni_init(
    WinTCNotificationSni* self
)
{
    WINTC_LOG_DEBUG(
        "taskband: tray sni hosting name %s",
        "org.kde.StatusNotifierWatcher"
    );

    g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "org.kde.StatusNotifierWatcher",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        NULL,
        on_name_acquired,
        on_name_lost,
        self,
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_sni_constructed(
    WINTC_UNUSED(GObject* object)
) {}

//
// CALLBACKS
//
static gboolean on_handle_register_status_notifier_item(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    const gchar*                  service,
    gpointer                      user_data
)
{
    WinTCNotificationSni* sni = WINTC_NOTIFICATION_SNI(user_data);

    //
    // Call in to register an item - we take in the parameter 'service' as the
    // DBus name for where to look for a /StatusNotifierItem object
    //

    if (!g_dbus_is_name(service))
    {
        g_dbus_method_invocation_return_error_literal(
            invocation,
            G_IO_ERROR,
            G_IO_ERROR_INVALID_ARGUMENT,
            "Bus name provided is unacceptable."
        );

        return FALSE;
    }

    //
    // FIXME: Spawn an item in our GUI
    //
    GtkWidget* notif_icon =
        wintc_ishext_ui_host_get_ext_widget(
            wintc_shext_ui_controller_get_ui_host(
                WINTC_SHEXT_UI_CONTROLLER(sni)
            ),
            WINTC_NOTIFAREA_HOSTEXT_ICON,
            WINTC_TYPE_NOTIF_AREA_ICON,
            sni
        );

    wintc_notif_area_icon_set_icon_name(
        WINTC_NOTIF_AREA_ICON(notif_icon),
        "preferences-desktop-locale"
    );

    WINTC_LOG_DEBUG("SNI: New service: %s", service);

    zwin_kde_status_notifier_watcher_complete_register_status_notifier_item(
        dbus_snw,
        invocation
    );

    return TRUE;
}

static gboolean on_handle_register_status_notifier_host(
    ZWinKdeStatusNotifierWatcher* dbus_snw,
    GDBusMethodInvocation*        invocation,
    WINTC_UNUSED(const gchar* service),
    WINTC_UNUSED(gpointer user_data)
)
{
    // FIXME: Implement this
    //
    zwin_kde_status_notifier_watcher_complete_register_status_notifier_host(
        dbus_snw,
        invocation
    );

    return TRUE;
}

static void on_name_acquired(
    GDBusConnection* connection,
    WINTC_UNUSED(const gchar* name),
    gpointer         user_data
)
{
    WinTCNotificationSni* sni = WINTC_NOTIFICATION_SNI(user_data);

    GError* error = NULL;

    // Create the DBus host
    //
    ZWinKdeStatusNotifierWatcher* dbus_snw =
        zwin_kde_status_notifier_watcher_skeleton_new();

    zwin_kde_status_notifier_watcher_set_is_status_notifier_host_registered(
        dbus_snw,
        TRUE
    );
    zwin_kde_status_notifier_watcher_set_registered_status_notifier_items(
        dbus_snw,
        NULL
    );
    zwin_kde_status_notifier_watcher_set_protocol_version(
        dbus_snw,
        0
    );

    g_signal_connect(
        dbus_snw,
        "handle-register-status-notifier-item",
        G_CALLBACK(on_handle_register_status_notifier_item),
        sni
    );
    g_signal_connect(
        dbus_snw,
        "handle-register-status-notifier-host",
        G_CALLBACK(on_handle_register_status_notifier_host),
        sni
    );

    if (
        !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(dbus_snw),
            connection,
            "/StatusNotifierWatcher",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        g_clear_object(&dbus_snw);
        return;
    }

    sni->dbus_snw = dbus_snw;
}

static void on_name_lost(
    WINTC_UNUSED(GDBusConnection* connection),
    WINTC_UNUSED(const gchar*     name),
    WINTC_UNUSED(gpointer         user_data)
)
{
    // FIXME: We should probably do something about this
}
