#include <glib.h>
#include <gtk/gtk.h>
#include <NetworkManager.h>
#include <wintc/comgtk.h>

#include "behaviour.h"
#include "network.h"

//
// STATIC DATA
//
GtkWidget* s_menu_network = NULL;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationNetworkClass
{
    WinTCNotificationBehaviourClass __parent__;
};

struct _WinTCNotificationNetwork
{
    WinTCNotificationBehaviour __parent__;

    // Network stuff
    //
    NMClient*           nm_client;
    NMActiveConnection* nm_primary_connection;
};

//
// FORWARD DECLARATIONS
//
static void wintc_notification_network_constructed(
    GObject* object
);

static void nm_update_primary_connection(
    WinTCNotificationNetwork* network,
    NMClient*                 nm_client
);

static void on_nm_client_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_client_notify_primary_connection(
    NMClient*   nm_client,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_widget_notif_button_press_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationNetwork,
    wintc_notification_network,
    WINTC_TYPE_NOTIFICATION_BEHAVIOUR
)

static void wintc_notification_network_class_init(
    WinTCNotificationNetworkClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_network_constructed;
}

static void wintc_notification_network_init(
    WINTC_UNUSED(WinTCNotificationNetwork* self)
)
{
    if (!s_menu_network)
    {
        GtkBuilder* builder =
            gtk_builder_new_from_resource(
                "/uk/oddmatics/wintc/taskband/menu-tray-nm.ui"
            );

        s_menu_network =
            GTK_WIDGET(
                g_object_ref(gtk_builder_get_object(builder, "menu"))
            );

        g_object_unref(builder);
    }
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_network_constructed(
    GObject* object
)
{
    WinTCNotificationBehaviour* behaviour =
        WINTC_NOTIFICATION_BEHAVIOUR(object);
    WinTCNotificationNetwork*   network =
        WINTC_NOTIFICATION_NETWORK(object);

    g_object_set(
        network,
        "icon-name", "network-offline",
        NULL
    );

    // Connect to NetworkManager
    //
    WINTC_LOG_DEBUG("Connecting to NM...");

    nm_client_new_async(
        NULL,
        (GAsyncReadyCallback) on_nm_client_ready,
        network
    );

    // Hook up signals for widget
    //
    g_signal_connect(
        behaviour->widget_notif,
        "button-press-event",
        G_CALLBACK(on_widget_notif_button_press_event),
        network
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCNotificationNetwork* wintc_notification_network_new(
    GtkWidget* widget_notif
)
{
    return WINTC_NOTIFICATION_NETWORK(
        g_object_new(
            WINTC_TYPE_NOTIFICATION_NETWORK,
            "widget-notif", widget_notif,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void nm_update_primary_connection(
    WinTCNotificationNetwork* network,
    NMClient*                 nm_client
)
{
    g_object_get(
        nm_client,
        "primary-connection", &(network->nm_primary_connection),
        NULL
    );

    if (!network->nm_primary_connection)
    {
        g_object_set(
            network,
            "icon-name", "network-offline",
            NULL
        );

        return;
    }

    // FIXME: Decide network type and stuff
    //
    g_object_set(
        network,
        "icon-name", "network-idle",
        NULL
    );
}

//
// CALLBACKS
//
static void on_nm_client_notify_primary_connection(
    NMClient* nm_client,
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer  user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    nm_update_primary_connection(network, nm_client);
}

static void on_nm_client_ready(
    WINTC_UNUSED(GObject* source_object),
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    // Finish up async connection
    //
    GError* error = NULL;

    network->nm_client = nm_client_new_finish(res, &error);

    if (!network->nm_client)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    WINTC_LOG_DEBUG("Connected to NM");

    // Monitor primary connection
    //
    nm_update_primary_connection(network, network->nm_client);

    g_signal_connect(
        network->nm_client,
        "notify::primary-connection",
        G_CALLBACK(on_nm_client_notify_primary_connection),
        network
    );
}

static void on_widget_notif_button_press_event(
    WINTC_UNUSED(GtkWidget* self),
    GdkEventButton* event,
    WINTC_UNUSED(gpointer user_data)
)
{
    if (event->button == 3)
    {
        gtk_menu_popup_at_pointer(
            GTK_MENU(s_menu_network),
            (GdkEvent*) event
        );
    }
}
