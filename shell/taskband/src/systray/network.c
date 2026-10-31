#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <net/if.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "../net/client.h"
#include "../net/device.h"
#include "../net/dialogs.h"
#include "../net/netwnd.h"
#include "../net/statwnd.h"
#include "../net/util.h"
#include "icon.h"
#include "network.h"

//
// PRIVATE CONSTANTS
//
#define NET_TRAY_ICON_SIZE 16

#define NET_ACQUIRING_FRAME_MS 300

#define NET_DEFAULT_ITEM_CSS_CLASS "wintc-default-item"

//
// PRIVATE STRUCTURES
//

// One tray icon per network adapter, the same as XP has one per connection
//
typedef struct _WinTCNetAdapter
{
    WinTCNotificationNetwork* owner;

    WinTCNetDevice*    device;
    WinTCNetDeviceKind kind;
    guint              index;
    gchar*             connection_name;

    GtkWidget* notif_icon;
    GtkWidget* wnd_networks;
    GtkWidget* wnd_status;

    const gchar* const* icon_names;
    guint               anim_step;
    guint               id_anim;

    gulong sigid_activity_changed;
    gulong sigid_button_press;
    gulong sigid_state_changed;
} WinTCNetAdapter;

//
// FORWARD DECLARATIONS
//
static void wintc_notification_network_constructed(
    GObject* object
);
static void wintc_notification_network_dispose(
    GObject* object
);

static void wintc_notification_network_add_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetDevice*           device
);
static WinTCNetAdapter* wintc_notification_network_find_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetDevice*           device
);
static void wintc_notification_network_popup_menu(
    WinTCNotificationNetwork* network,
    WinTCNetAdapter*          adapter,
    GdkEvent*                 event
);
static void wintc_notification_network_remove_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetAdapter*          adapter
);
static void wintc_notification_network_sync_devices(
    WinTCNotificationNetwork* network
);

static void net_adapter_do_default_action(
    WinTCNetAdapter* adapter
);
static void net_adapter_show_networks(
    WinTCNetAdapter* adapter
);
static void net_adapter_show_status(
    WinTCNetAdapter* adapter
);
static void net_adapter_update(
    WinTCNetAdapter* adapter
);
static void net_adapter_update_icon(
    WinTCNetAdapter* adapter
);

static gint net_compare_devices(
    gconstpointer a,
    gconstpointer b
);
static gboolean net_state_has_status(
    WinTCNetState state
);

static void action_disable(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_enable(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_firewall(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_open_connections(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_repair(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_status(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_view_networks(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void on_client_devices_changed(
    WinTCNetClient* client,
    gpointer        user_data
);
static void on_device_action_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_device_activity_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_device_state_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_icon_theme_changed(
    GtkIconTheme* self,
    gpointer      user_data
);
static gboolean on_notif_icon_button_press_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);
static void on_status_window_view_networks(
    GtkWidget* self,
    gpointer   user_data
);
static gboolean on_timeout_anim(
    gpointer user_data
);

//
// STATIC DATA
//
static GActionEntry S_ACTIONS[] = {
    {
        .name           = "disable",
        .activate       = action_disable,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "enable",
        .activate       = action_enable,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "firewall",
        .activate       = action_firewall,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "open-connections",
        .activate       = action_open_connections,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "repair",
        .activate       = action_repair,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "status",
        .activate       = action_status,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "view-networks",
        .activate       = action_view_networks,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

// The yellow ball moves away from the computer and back again
//
static const guint S_ACQUIRING_FRAMES[] = { 0, 1, 2, 1 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationNetwork
{
    WinTCShextUIController __parent__;

    // UI
    //
    GtkWidget* menu_connected;
    GtkWidget* menu_disabled;
    GtkWidget* menu_lan_disconnected;
    GtkWidget* menu_wlan_disconnected;
    GtkWidget* item_status;
    GtkWidget* item_view_networks;
    GtkWidget* sep_view_networks;

    GSimpleActionGroup* action_group;
    WinTCNetAdapter*    menu_adapter;

    GtkWidget* spare_icon;

    // Network stuff
    //
    WinTCNetClient* client;
    GList*          adapters;

    gulong sigid_devices_changed;
    gulong sigid_theme_changed;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationNetwork,
    wintc_notification_network,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_notification_network_class_init(
    WinTCNotificationNetworkClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_network_constructed;
    object_class->dispose     = wintc_notification_network_dispose;
}

static void wintc_notification_network_init(
    WinTCNotificationNetwork* self
)
{
    GtkBuilder* builder;

    wintc_net_ensure_styles();

    // Set up the context menus
    //
    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/menu-tray-net.ui"
        );

    wintc_builder_get_objects(
        builder,
        "menu-connected",         &(self->menu_connected),
        "menu-disabled",          &(self->menu_disabled),
        "menu-lan-disconnected",  &(self->menu_lan_disconnected),
        "menu-wlan-disconnected", &(self->menu_wlan_disconnected),
        "item-status",            &(self->item_status),
        "item-view-networks",     &(self->item_view_networks),
        "sep-view-networks",      &(self->sep_view_networks),
        NULL
    );

    g_object_ref_sink(self->menu_connected);
    g_object_ref_sink(self->menu_disabled);
    g_object_ref_sink(self->menu_lan_disconnected);
    g_object_ref_sink(self->menu_wlan_disconnected);

    g_object_unref(builder);

    self->action_group = g_simple_action_group_new();

    g_action_map_add_action_entries(
        G_ACTION_MAP(self->action_group),
        S_ACTIONS,
        G_N_ELEMENTS(S_ACTIONS),
        self
    );

    GtkWidget* menus[] = {
        self->menu_connected,
        self->menu_disabled,
        self->menu_lan_disconnected,
        self->menu_wlan_disconnected
    };

    for (gsize i = 0; i < G_N_ELEMENTS(menus); i++)
    {
        gtk_widget_insert_action_group(
            menus[i],
            "net",
            G_ACTION_GROUP(self->action_group)
        );
    }

    // Status is what a click on a connected icon does, so it's in bold
    //
    wintc_widget_add_style_class(
        self->item_status,
        NET_DEFAULT_ITEM_CSS_CLASS
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_network_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_notification_network_parent_class))
        ->constructed(object);

    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(object);

    // Grab an icon straight away so that it lands in the right spot in the
    // tray, it stays hidden until an adapter is found
    //
    network->spare_icon =
        wintc_ishext_ui_host_get_ext_widget(
            wintc_shext_ui_controller_get_ui_host(
                WINTC_SHEXT_UI_CONTROLLER(object)
            ),
            WINTC_NOTIFAREA_HOSTEXT_ICON,
            WINTC_TYPE_NOTIF_AREA_ICON,
            object
        );

    g_object_add_weak_pointer(
        G_OBJECT(network->spare_icon),
        (gpointer*) &(network->spare_icon)
    );

    gtk_widget_set_no_show_all(network->spare_icon, TRUE);
    gtk_widget_hide(network->spare_icon);

    network->sigid_theme_changed =
        g_signal_connect(
            gtk_icon_theme_get_default(),
            "changed",
            G_CALLBACK(on_icon_theme_changed),
            network
        );

    // Adapters come and go with the backend, see net/client.h
    //
    network->client = wintc_net_client_new();

    network->sigid_devices_changed =
        g_signal_connect(
            network->client,
            "devices-changed",
            G_CALLBACK(on_client_devices_changed),
            network
        );

    wintc_notification_network_sync_devices(network);
}

static void wintc_notification_network_dispose(
    GObject* object
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(object);

    g_clear_signal_handler(
        &(network->sigid_theme_changed),
        gtk_icon_theme_get_default()
    );

    while (network->adapters)
    {
        wintc_notification_network_remove_adapter(
            network,
            (WinTCNetAdapter*) network->adapters->data
        );
    }

    if (network->client)
    {
        g_clear_signal_handler(
            &(network->sigid_devices_changed),
            network->client
        );
    }

    if (network->spare_icon)
    {
        g_object_remove_weak_pointer(
            G_OBJECT(network->spare_icon),
            (gpointer*) &(network->spare_icon)
        );

        network->spare_icon = NULL;
    }

    GtkWidget** menus[] = {
        &(network->menu_connected),
        &(network->menu_disabled),
        &(network->menu_lan_disconnected),
        &(network->menu_wlan_disconnected)
    };

    for (gsize i = 0; i < G_N_ELEMENTS(menus); i++)
    {
        if (*(menus[i]))
        {
            gtk_widget_destroy(*(menus[i]));
            g_clear_object(menus[i]);
        }
    }

    g_clear_object(&(network->action_group));
    g_clear_object(&(network->client));

    (G_OBJECT_CLASS(wintc_notification_network_parent_class))
        ->dispose(object);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_notification_network_add_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetDevice*           device
)
{
    WinTCNetAdapter* adapter = g_new0(WinTCNetAdapter, 1);

    adapter->owner  = network;
    adapter->device = g_object_ref(device);
    adapter->kind   = wintc_net_device_get_kind(adapter->device);

    // XP numbers connections of each kind, pick the lowest number not in
    // use
    //
    guint    index = 0;
    gboolean index_in_use;

    do
    {
        index_in_use = FALSE;

        for (GList* iter = network->adapters; iter; iter = iter->next)
        {
            WinTCNetAdapter* other = (WinTCNetAdapter*) iter->data;

            if (other->kind == adapter->kind && other->index == index)
            {
                index_in_use = TRUE;
                index++;
                break;
            }
        }
    }
    while (index_in_use);

    adapter->index           = index;
    adapter->connection_name =
        wintc_net_get_connection_name(adapter->kind, index);

    // Use the icon we reserved at startup if possible, otherwise put the new
    // icon next to the existing ones
    //
    if (network->spare_icon)
    {
        adapter->notif_icon = network->spare_icon;

        g_object_remove_weak_pointer(
            G_OBJECT(network->spare_icon),
            (gpointer*) &(network->spare_icon)
        );

        network->spare_icon = NULL;
    }
    else
    {
        adapter->notif_icon =
            wintc_ishext_ui_host_get_ext_widget(
                wintc_shext_ui_controller_get_ui_host(
                    WINTC_SHEXT_UI_CONTROLLER(network)
                ),
                WINTC_NOTIFAREA_HOSTEXT_ICON,
                WINTC_TYPE_NOTIF_AREA_ICON,
                network
            );

        if (network->adapters)
        {
            WinTCNetAdapter* last =
                (WinTCNetAdapter*) g_list_last(network->adapters)->data;

            GtkWidget* box = gtk_widget_get_parent(adapter->notif_icon);

            if (
                last->notif_icon &&
                GTK_IS_BOX(box)   &&
                gtk_widget_get_parent(last->notif_icon) == box
            )
            {
                gboolean    expand;
                gboolean    fill;
                guint       padding;
                GtkPackType pack_type;
                gint        position;

                gtk_box_query_child_packing(
                    GTK_BOX(box),
                    last->notif_icon,
                    &expand,
                    &fill,
                    &padding,
                    &pack_type
                );
                gtk_container_child_get(
                    GTK_CONTAINER(box),
                    last->notif_icon,
                    "position", &position,
                    NULL
                );

                gtk_box_set_child_packing(
                    GTK_BOX(box),
                    adapter->notif_icon,
                    expand,
                    fill,
                    padding,
                    pack_type
                );
                gtk_box_reorder_child(
                    GTK_BOX(box),
                    adapter->notif_icon,
                    position + 1
                );
            }
        }
    }

    g_object_add_weak_pointer(
        G_OBJECT(adapter->notif_icon),
        (gpointer*) &(adapter->notif_icon)
    );

    gtk_widget_set_no_show_all(adapter->notif_icon, FALSE);
    gtk_widget_show(adapter->notif_icon);

    adapter->sigid_button_press =
        g_signal_connect(
            adapter->notif_icon,
            "button-press-event",
            G_CALLBACK(on_notif_icon_button_press_event),
            adapter
        );
    adapter->sigid_state_changed =
        g_signal_connect(
            adapter->device,
            "state-changed",
            G_CALLBACK(on_device_state_changed),
            adapter
        );
    adapter->sigid_activity_changed =
        g_signal_connect(
            adapter->device,
            "activity-changed",
            G_CALLBACK(on_device_activity_changed),
            adapter
        );

    network->adapters = g_list_append(network->adapters, adapter);

    WINTC_LOG_DEBUG(
        "net: added adapter %s as '%s'",
        wintc_net_device_get_interface(device),
        adapter->connection_name
    );

    net_adapter_update(adapter);
}

static WinTCNetAdapter* wintc_notification_network_find_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetDevice*           device
)
{
    for (GList* iter = network->adapters; iter; iter = iter->next)
    {
        WinTCNetAdapter* adapter = (WinTCNetAdapter*) iter->data;

        if (adapter->device == device)
        {
            return adapter;
        }
    }

    return NULL;
}

static void wintc_notification_network_popup_menu(
    WinTCNotificationNetwork* network,
    WinTCNetAdapter*          adapter,
    GdkEvent*                 event
)
{
    WinTCNetState state    = wintc_net_device_get_state(adapter->device);
    gboolean      wireless = adapter->kind == WINTC_NET_DEVICE_KIND_WIRELESS;
    GtkWidget*    menu;

    network->menu_adapter = adapter;

    // XP has a different menu depending on the type of connection, and
    // whether it's connected
    //
    if (state == WINTC_NET_STATE_DISABLED)
    {
        menu = network->menu_disabled;
    }
    else if (net_state_has_status(state))
    {
        menu = network->menu_connected;

        gtk_widget_set_visible(network->item_view_networks, wireless);
        gtk_widget_set_visible(network->sep_view_networks,  wireless);
    }
    else
    {
        menu =
            wireless ?
                network->menu_wlan_disconnected :
                network->menu_lan_disconnected;
    }

    struct
    {
        const gchar* name;
        gboolean     enabled;
    } action_states[] = {
        { "disable",          state != WINTC_NET_STATE_DISABLED             },
        { "enable",           state == WINTC_NET_STATE_DISABLED             },
        { "status",           net_state_has_status(state)                   },
        { "repair",           wintc_net_device_is_connected(adapter->device) },
        { "view-networks",    wireless                                      },
        { "firewall",         wintc_net_tool_exists("gufw")                 },
        { "open-connections", wintc_net_tool_exists("nm-connection-editor") }
    };

    for (gsize i = 0; i < G_N_ELEMENTS(action_states); i++)
    {
        g_simple_action_set_enabled(
            G_SIMPLE_ACTION(
                g_action_map_lookup_action(
                    G_ACTION_MAP(network->action_group),
                    action_states[i].name
                )
            ),
            action_states[i].enabled
        );
    }

    gtk_menu_popup_at_pointer(GTK_MENU(menu), event);
}

static void wintc_notification_network_remove_adapter(
    WinTCNotificationNetwork* network,
    WinTCNetAdapter*          adapter
)
{
    WINTC_LOG_DEBUG("net: removing adapter '%s'", adapter->connection_name);

    if (network->menu_adapter == adapter)
    {
        gtk_menu_popdown(GTK_MENU(network->menu_connected));
        gtk_menu_popdown(GTK_MENU(network->menu_disabled));
        gtk_menu_popdown(GTK_MENU(network->menu_lan_disconnected));
        gtk_menu_popdown(GTK_MENU(network->menu_wlan_disconnected));

        network->menu_adapter = NULL;
    }

    // Windows clear their pointers via weak refs when destroyed
    //
    if (adapter->wnd_networks)
    {
        gtk_widget_destroy(adapter->wnd_networks);
    }

    if (adapter->wnd_status)
    {
        gtk_widget_destroy(adapter->wnd_status);
    }

    g_clear_handle_id(&(adapter->id_anim), g_source_remove);

    g_clear_signal_handler(
        &(adapter->sigid_state_changed),
        adapter->device
    );
    g_clear_signal_handler(
        &(adapter->sigid_activity_changed),
        adapter->device
    );

    network->adapters = g_list_remove(network->adapters, adapter);

    if (adapter->notif_icon)
    {
        g_clear_signal_handler(
            &(adapter->sigid_button_press),
            adapter->notif_icon
        );

        g_object_remove_weak_pointer(
            G_OBJECT(adapter->notif_icon),
            (gpointer*) &(adapter->notif_icon)
        );

        // Keep the last icon around so that a returning adapter gets its
        // spot in the tray back
        //
        if (!(network->adapters) && !(network->spare_icon))
        {
            network->spare_icon = adapter->notif_icon;

            g_object_add_weak_pointer(
                G_OBJECT(network->spare_icon),
                (gpointer*) &(network->spare_icon)
            );

            gtk_widget_set_tooltip_text(network->spare_icon, NULL);
            gtk_widget_hide(network->spare_icon);
        }
        else
        {
            gtk_widget_destroy(adapter->notif_icon);
        }
    }

    g_object_unref(adapter->device);

    g_free(adapter->connection_name);
    g_free(adapter);
}

static void wintc_notification_network_sync_devices(
    WinTCNotificationNetwork* network
)
{
    GPtrArray* devices = wintc_net_client_get_devices(network->client);

    // Remove adapters that went away...
    //
    GList* iter = network->adapters;

    while (iter)
    {
        WinTCNetAdapter* adapter = (WinTCNetAdapter*) iter->data;

        iter = iter->next;

        if (!g_ptr_array_find(devices, adapter->device, NULL))
        {
            wintc_notification_network_remove_adapter(network, adapter);
        }
    }

    // ...and add new ones, in adapter order so the numbering stays the same
    // across restarts
    //
    GPtrArray* shown = g_ptr_array_new();

    for (guint i = 0; i < devices->len; i++)
    {
        g_ptr_array_add(shown, g_ptr_array_index(devices, i));
    }

    g_ptr_array_sort(shown, net_compare_devices);

    for (guint i = 0; i < shown->len; i++)
    {
        WinTCNetDevice* device = g_ptr_array_index(shown, i);

        if (!wintc_notification_network_find_adapter(network, device))
        {
            wintc_notification_network_add_adapter(network, device);
        }
    }

    g_ptr_array_free(shown, TRUE);
}

static void net_adapter_do_default_action(
    WinTCNetAdapter* adapter
)
{
    WinTCNetState state = wintc_net_device_get_state(adapter->device);

    if (net_state_has_status(state))
    {
        net_adapter_show_status(adapter);
        return;
    }

    if (adapter->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        if (state != WINTC_NET_STATE_DISABLED)
        {
            net_adapter_show_networks(adapter);
        }

        return;
    }

    // Same as trying to connect an unplugged connection in XP
    //
    if (state == WINTC_NET_STATE_UNAVAILABLE)
    {
        wintc_messagebox_show(
            NULL,
            _("It is not possible to connect at this time. No network was detected. You may need to plug in your network cable to complete the connection."),
            adapter->connection_name,
            WINTC_BUTTONS_OK,
            WINTC_MESSAGE_INFORMATION
        );
    }
}

static void net_adapter_show_networks(
    WinTCNetAdapter* adapter
)
{
    if (adapter->kind != WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        return;
    }

    if (!(adapter->wnd_networks))
    {
        adapter->wnd_networks =
            wintc_wlan_networks_window_new(
                adapter->device,
                adapter->connection_name
            );

        g_object_add_weak_pointer(
            G_OBJECT(adapter->wnd_networks),
            (gpointer*) &(adapter->wnd_networks)
        );

        gtk_widget_show(adapter->wnd_networks);
    }

    wintc_focus_window(GTK_WINDOW(adapter->wnd_networks));
}

static void net_adapter_show_status(
    WinTCNetAdapter* adapter
)
{
    if (!(adapter->wnd_status))
    {
        adapter->wnd_status =
            wintc_net_status_window_new(
                adapter->device,
                adapter->connection_name
            );

        g_object_add_weak_pointer(
            G_OBJECT(adapter->wnd_status),
            (gpointer*) &(adapter->wnd_status)
        );

        g_signal_connect(
            adapter->wnd_status,
            "view-networks",
            G_CALLBACK(on_status_window_view_networks),
            adapter
        );

        gtk_widget_show(adapter->wnd_status);
    }

    wintc_focus_window(GTK_WINDOW(adapter->wnd_status));
}

static void net_adapter_update(
    WinTCNetAdapter* adapter
)
{
    if (!(adapter->notif_icon))
    {
        return;
    }

    WinTCNetState state = wintc_net_device_get_state(adapter->device);

    // The yellow ball runs while waiting for an address
    //
    if (
        state == WINTC_NET_STATE_CONNECTING ||
        state == WINTC_NET_STATE_ACQUIRING
    )
    {
        if (!(adapter->id_anim))
        {
            adapter->anim_step = 0;
            adapter->id_anim   =
                g_timeout_add(
                    NET_ACQUIRING_FRAME_MS,
                    on_timeout_anim,
                    adapter
                );
        }
    }
    else
    {
        g_clear_handle_id(&(adapter->id_anim), g_source_remove);
    }

    net_adapter_update_icon(adapter);

    // Tooltip, as in XP:
    //   Wireless Network Connection (linksys)
    //   Speed: 54.0 Mbps
    //   Signal Strength: Excellent
    //   Status: Connected
    //
    //   Local Area Connection
    //   A network cable is unplugged.
    //
    GString* tooltip = g_string_new(adapter->connection_name);
    gboolean active  =
        net_state_has_status(state) &&
        state != WINTC_NET_STATE_DISCONNECTING;

    if (adapter->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        WinTCWlanNetwork* active_network =
            wintc_net_device_get_active_network(adapter->device);

        if (active && active_network)
        {
            g_string_append_printf(tooltip, " (%s)", active_network->ssid);

            gchar* speed =
                wintc_net_speed_to_text(
                    wintc_net_device_get_speed(adapter->device)
                );

            g_string_append_c(tooltip, '\n');
            g_string_append_printf(tooltip, _("Speed: %s"), speed);
            g_string_append_c(tooltip, '\n');
            g_string_append_printf(
                tooltip,
                _("Signal Strength: %s"),
                wintc_wlan_signal_to_text(active_network->signal)
            );
            g_string_append_c(tooltip, '\n');
            g_string_append_printf(
                tooltip,
                _("Status: %s"),
                wintc_net_state_to_text(adapter->kind, state)
            );

            g_free(speed);
        }
        else if (state == WINTC_NET_STATE_DISCONNECTED)
        {
            g_string_append_c(tooltip, '\n');
            g_string_append(
                tooltip,
                _("Not connected. Right-click here for more options.")
            );
        }
        else
        {
            g_string_append_c(tooltip, '\n');
            g_string_append(
                tooltip,
                wintc_net_state_to_text(adapter->kind, state)
            );
        }
    }
    else
    {
        if (active)
        {
            gchar* speed =
                wintc_net_speed_to_text(
                    wintc_net_device_get_speed(adapter->device)
                );

            g_string_append_c(tooltip, '\n');
            g_string_append_printf(tooltip, _("Speed: %s"), speed);
            g_string_append_c(tooltip, '\n');
            g_string_append_printf(
                tooltip,
                _("Status: %s"),
                wintc_net_state_to_text(adapter->kind, state)
            );

            g_free(speed);
        }
        else if (state == WINTC_NET_STATE_UNAVAILABLE)
        {
            g_string_append_c(tooltip, '\n');
            g_string_append(tooltip, _("A network cable is unplugged."));
        }
        else
        {
            g_string_append_c(tooltip, '\n');
            g_string_append(
                tooltip,
                wintc_net_state_to_text(adapter->kind, state)
            );
        }
    }

    gtk_widget_set_tooltip_text(adapter->notif_icon, tooltip->str);

    g_string_free(tooltip, TRUE);
}

static void net_adapter_update_icon(
    WinTCNetAdapter* adapter
)
{
    if (!(adapter->notif_icon))
    {
        return;
    }

    const gchar* const* icon_names =
        wintc_net_get_tray_icon_names(
            adapter->kind,
            wintc_net_device_get_state(adapter->device),
            wintc_net_device_get_activity(adapter->device),
            S_ACQUIRING_FRAMES[adapter->anim_step]
        );

    // Activity changes a couple of times a second, don't reload the same
    // icon each time
    //
    if (icon_names == adapter->icon_names)
    {
        return;
    }

    GdkPixbuf* pixbuf = wintc_net_load_icon(icon_names, NET_TRAY_ICON_SIZE);

    wintc_notif_area_icon_set_icon_pixbuf(
        WINTC_NOTIF_AREA_ICON(adapter->notif_icon),
        pixbuf
    );

    g_clear_object(&pixbuf);

    adapter->icon_names = icon_names;
}

static gint net_compare_devices(
    gconstpointer a,
    gconstpointer b
)
{
    const gchar* iface_a =
        wintc_net_device_get_interface(*((WinTCNetDevice**) a));
    const gchar* iface_b =
        wintc_net_device_get_interface(*((WinTCNetDevice**) b));

    guint index_a = if_nametoindex(iface_a);
    guint index_b = if_nametoindex(iface_b);

    if (index_a == index_b)
    {
        return g_strcmp0(iface_a, iface_b);
    }

    // Interfaces that have gone away (index 0) go last
    //
    if (!index_a || !index_b)
    {
        return index_a ? -1 : 1;
    }

    return index_a < index_b ? -1 : 1;
}

static gboolean net_state_has_status(
    WinTCNetState state
)
{
    switch (state)
    {
        case WINTC_NET_STATE_CONNECTING:
        case WINTC_NET_STATE_ACQUIRING:
        case WINTC_NET_STATE_CONNECTED:
        case WINTC_NET_STATE_LIMITED:
        case WINTC_NET_STATE_DISCONNECTING:
            return TRUE;

        default:
            return FALSE;
    }
}

//
// CALLBACKS
//
static void action_disable(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    WINTC_RETURN_IF_FAIL(network->menu_adapter);

    wintc_net_device_set_enabled_async(
        network->menu_adapter->device,
        FALSE,
        on_device_action_finished,
        NULL
    );
}

static void action_enable(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    WINTC_RETURN_IF_FAIL(network->menu_adapter);

    wintc_net_device_set_enabled_async(
        network->menu_adapter->device,
        TRUE,
        on_device_action_finished,
        NULL
    );
}

static void action_firewall(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    WINTC_UNUSED(gpointer       user_data)
)
{
    // FIXME: There is no Windows Firewall applet yet, use the firewall tool
    //        if there is one
    //
    wintc_net_launch_tool("gufw", NULL);
}

static void action_open_connections(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    WINTC_UNUSED(gpointer       user_data)
)
{
    // FIXME: Should be the Network Connections folder
    //
    wintc_net_launch_tool("nm-connection-editor", NULL);
}

static void action_repair(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    WINTC_RETURN_IF_FAIL(network->menu_adapter);

    wintc_net_repair_connection(
        network->menu_adapter->device,
        network->menu_adapter->connection_name,
        NULL
    );
}

static void action_status(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    WINTC_RETURN_IF_FAIL(network->menu_adapter);

    net_adapter_show_status(network->menu_adapter);
}

static void action_view_networks(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    WINTC_RETURN_IF_FAIL(network->menu_adapter);

    net_adapter_show_networks(network->menu_adapter);
}

static void on_client_devices_changed(
    WINTC_UNUSED(WinTCNetClient* client),
    gpointer user_data
)
{
    wintc_notification_network_sync_devices(
        WINTC_NOTIFICATION_NETWORK(user_data)
    );
}

static void on_device_action_finished(
    GObject*      source_object,
    GAsyncResult* res,
    WINTC_UNUSED(gpointer user_data)
)
{
    GError* error = NULL;

    if (
        !wintc_net_device_action_finish(
            WINTC_NET_DEVICE(source_object),
            res,
            &error
        )
    )
    {
        if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
            g_clear_error(&error);
        }
        else
        {
            wintc_display_error_and_clear(&error, NULL);
        }
    }
}

static void on_device_activity_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    net_adapter_update_icon((WinTCNetAdapter*) user_data);
}

static void on_device_state_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    net_adapter_update((WinTCNetAdapter*) user_data);
}

static void on_icon_theme_changed(
    WINTC_UNUSED(GtkIconTheme* self),
    gpointer user_data
)
{
    WinTCNotificationNetwork* network = WINTC_NOTIFICATION_NETWORK(user_data);

    for (GList* iter = network->adapters; iter; iter = iter->next)
    {
        WinTCNetAdapter* adapter = (WinTCNetAdapter*) iter->data;

        adapter->icon_names = NULL;

        net_adapter_update_icon(adapter);
    }
}

static gboolean on_notif_icon_button_press_event(
    WINTC_UNUSED(GtkWidget* self),
    GdkEventButton* event,
    gpointer        user_data
)
{
    WinTCNetAdapter* adapter = (WinTCNetAdapter*) user_data;

    // Ignore the extra events generated for double clicks
    //
    if (event->type != GDK_BUTTON_PRESS)
    {
        return TRUE;
    }

    switch (event->button)
    {
        case GDK_BUTTON_PRIMARY:
            net_adapter_do_default_action(adapter);
            return TRUE;

        case GDK_BUTTON_SECONDARY:
            wintc_notification_network_popup_menu(
                adapter->owner,
                adapter,
                (GdkEvent*) event
            );
            return TRUE;

        default:
            return FALSE;
    }
}

static void on_status_window_view_networks(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    net_adapter_show_networks((WinTCNetAdapter*) user_data);
}

static gboolean on_timeout_anim(
    gpointer user_data
)
{
    WinTCNetAdapter* adapter = (WinTCNetAdapter*) user_data;

    adapter->anim_step =
        (adapter->anim_step + 1) % G_N_ELEMENTS(S_ACQUIRING_FRAMES);

    net_adapter_update_icon(adapter);

    return G_SOURCE_CONTINUE;
}
