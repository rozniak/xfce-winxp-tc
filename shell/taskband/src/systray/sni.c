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
}

static void wintc_notification_sni_init(
    WinTCNotificationSni* self
)
{
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
    GObject* object
)
{
}

//
// CALLBACKS
//
static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
)
{
    WinTCNotificationSni* notif_sni = WINTC_NOTIFICATION_SNI(user_data);

    // Create the DBus host
    //
    ZWinKdeStatusNotifierWatcher* dbus_snw =
        zwin_kde_status_notifier_watcher_new();
}

static void on_name_lost(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
)
{
}
