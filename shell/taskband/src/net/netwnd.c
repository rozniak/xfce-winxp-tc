#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <string.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "device.h"
#include "dialogs.h"
#include "netwnd.h"
#include "util.h"

//
// PRIVATE CONSTANTS
//
#define WLAN_CONNECTED_LINGER_MS 1000

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_DEVICE,
    PROP_CONNECTION_NAME,
    N_PROPERTIES
};

typedef enum
{
    WLAN_TASK_NONE = 0,
    WLAN_TASK_REFRESH,
    WLAN_TASK_SETUP,
    WLAN_TASK_PREFERRED_ORDER,
    WLAN_TASK_ADVANCED
} WinTCWlanTask;

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCWlanTaskDef
{
    const gchar*  label;
    const gchar*  icon_names[4];
    WinTCWlanTask task;
} WinTCWlanTaskDef;

//
// FORWARD DECLARATIONS
//
static void wintc_wlan_networks_window_constructed(
    GObject* object
);
static void wintc_wlan_networks_window_dispose(
    GObject* object
);
static void wintc_wlan_networks_window_finalize(
    GObject* object
);
static void wintc_wlan_networks_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_wlan_networks_window_activate_selected(
    WinTCWlanNetworksWindow* wnd
);
static void wintc_wlan_networks_window_add_task_group(
    WinTCWlanNetworksWindow* wnd,
    const gchar*             heading,
    const WinTCWlanTaskDef*  tasks,
    gsize                    n_tasks
);
static void wintc_wlan_networks_window_begin_connect(
    WinTCWlanNetworksWindow* wnd,
    WinTCWlanNetwork*        network
);
static GtkWidget* wintc_wlan_networks_window_create_row(
    WinTCWlanNetworksWindow* wnd,
    WinTCWlanNetwork*        network
);
static void wintc_wlan_networks_window_disconnect(
    WinTCWlanNetworksWindow* wnd,
    WinTCWlanNetwork*        network
);
static void wintc_wlan_networks_window_do_connect(
    WinTCWlanNetworksWindow* wnd,
    const gchar*             key
);
static void wintc_wlan_networks_window_show_key_dialog(
    WinTCWlanNetworksWindow* wnd
);
static void wintc_wlan_networks_window_update_button(
    WinTCWlanNetworksWindow* wnd
);
static void wintc_wlan_networks_window_update_list(
    WinTCWlanNetworksWindow* wnd
);

static void on_button_connect_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_device_action_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_device_connect_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_device_networks_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_device_state_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_dialog_key_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
);
static void on_dialog_progress_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
);
static void on_dialog_unsecured_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
);
static void on_list_networks_row_activated(
    GtkListBox*    self,
    GtkListBoxRow* row,
    gpointer       user_data
);
static void on_list_networks_row_selected(
    GtkListBox*    self,
    GtkListBoxRow* row,
    gpointer       user_data
);
static gboolean on_task_activate_link(
    GtkLinkButton* self,
    gpointer       user_data
);
static gboolean on_timeout_close_progress(
    gpointer user_data
);
static void on_window_destroy(
    GtkWidget* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_wlan_networks_window_properties[N_PROPERTIES] = {
    0
};

static GQuark S_QUARK_TASK_ID = 0;

static const WinTCWlanTaskDef S_NETWORK_TASKS[] = {
    {
        N_("Refresh network list"),
        { "network-wireless-refresh", "view-refresh", NULL },
        WLAN_TASK_REFRESH
    },
    {
        N_("Set up a wireless network for a home or small office"),
        { "network-wireless-setup", "network-wireless", NULL },
        WLAN_TASK_SETUP
    }
};

static const WinTCWlanTaskDef S_RELATED_TASKS[] = {
    {
        N_("Learn about wireless networking"),
        { "dialog-information", "help-browser", NULL },
        WLAN_TASK_NONE
    },
    {
        N_("Change the order of preferred networks"),
        { "network-wireless-preferred", "starred", NULL },
        WLAN_TASK_PREFERRED_ORDER
    },
    {
        N_("Change advanced settings"),
        { "network-wireless-advanced", "preferences-system", NULL },
        WLAN_TASK_ADVANCED
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWlanNetworksWindow
{
    GtkWindow __parent__;

    WinTCNetDevice* device;
    gchar*          connection_name;
    gboolean        destroyed;

    gulong sigid_networks_changed;
    gulong sigid_state_changed;

    // UI
    //
    GtkWidget* box_tasks;
    GtkWidget* button_connect;
    GtkWidget* img_message;
    GtkWidget* label_message;
    GtkWidget* list_networks;
    GtkWidget* stack_list;

    gboolean rebuilding;
    gchar*   selected_ssid;

    // Connection attempt in progress
    //
    GtkWidget*        dialog_progress;
    GCancellable*     cancellable_connect;
    gchar*            pending_ssid;
    WinTCWlanSecurity pending_security;
    gboolean          pending_has_profile;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWlanNetworksWindow,
    wintc_wlan_networks_window,
    GTK_TYPE_WINDOW
)

static void wintc_wlan_networks_window_class_init(
    WinTCWlanNetworksWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_wlan_networks_window_constructed;
    object_class->dispose      = wintc_wlan_networks_window_dispose;
    object_class->finalize     = wintc_wlan_networks_window_finalize;
    object_class->set_property = wintc_wlan_networks_window_set_property;

    wintc_wlan_networks_window_properties[PROP_DEVICE] =
        g_param_spec_object(
            "device",
            "Device",
            "The wireless network adapter.",
            WINTC_TYPE_NET_DEVICE,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );
    wintc_wlan_networks_window_properties[PROP_CONNECTION_NAME] =
        g_param_spec_string(
            "connection-name",
            "ConnectionName",
            "The display name of the wireless network connection.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_wlan_networks_window_properties
    );

    S_QUARK_TASK_ID = g_quark_from_static_string("wintc-wlan-task-id");
}

static void wintc_wlan_networks_window_init(
    WinTCWlanNetworksWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box = NULL;

    wintc_net_ensure_styles();

    gtk_window_set_default_size(GTK_WINDOW(self), 640, 460);
    gtk_window_set_icon_name(GTK_WINDOW(self), "network-wireless");
    gtk_window_set_position(GTK_WINDOW(self), GTK_WIN_POS_CENTER);

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-wlan-networks");

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/wlan-netwnd.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-box",       &main_box,
        "box-tasks",      &(self->box_tasks),
        "button-connect", &(self->button_connect),
        "img-message",    &(self->img_message),
        "label-message",  &(self->label_message),
        "list-networks",  &(self->list_networks),
        "stack-list",     &(self->stack_list),
        NULL
    );

    gtk_container_add(GTK_CONTAINER(self), main_box);

    g_object_unref(builder);

    // Task pane
    //
    wintc_wlan_networks_window_add_task_group(
        self,
        _("Network Tasks"),
        S_NETWORK_TASKS,
        G_N_ELEMENTS(S_NETWORK_TASKS)
    );
    wintc_wlan_networks_window_add_task_group(
        self,
        _("Related Tasks"),
        S_RELATED_TASKS,
        G_N_ELEMENTS(S_RELATED_TASKS)
    );

    // Signals
    //
    g_signal_connect(
        self->button_connect,
        "clicked",
        G_CALLBACK(on_button_connect_clicked),
        self
    );
    g_signal_connect(
        self->list_networks,
        "row-activated",
        G_CALLBACK(on_list_networks_row_activated),
        self
    );
    g_signal_connect(
        self->list_networks,
        "row-selected",
        G_CALLBACK(on_list_networks_row_selected),
        self
    );
    g_signal_connect(
        self,
        "destroy",
        G_CALLBACK(on_window_destroy),
        NULL
    );

    gtk_window_set_default(GTK_WINDOW(self), self->button_connect);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_wlan_networks_window_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_wlan_networks_window_parent_class))
        ->constructed(object);

    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(object);

    gtk_window_set_title(GTK_WINDOW(wnd), wnd->connection_name);

    wnd->sigid_networks_changed =
        g_signal_connect(
            wnd->device,
            "networks-changed",
            G_CALLBACK(on_device_networks_changed),
            wnd
        );
    wnd->sigid_state_changed =
        g_signal_connect(
            wnd->device,
            "state-changed",
            G_CALLBACK(on_device_state_changed),
            wnd
        );

    // XP scans for networks whenever the window is opened
    //
    wintc_net_device_request_scan(wnd->device);

    wintc_wlan_networks_window_update_list(wnd);
}

static void wintc_wlan_networks_window_dispose(
    GObject* object
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(object);

    if (wnd->device)
    {
        g_clear_signal_handler(&(wnd->sigid_networks_changed), wnd->device);
        g_clear_signal_handler(&(wnd->sigid_state_changed),    wnd->device);
    }

    if (wnd->cancellable_connect)
    {
        g_cancellable_cancel(wnd->cancellable_connect);
    }

    g_clear_object(&(wnd->cancellable_connect));
    g_clear_object(&(wnd->device));

    (G_OBJECT_CLASS(wintc_wlan_networks_window_parent_class))
        ->dispose(object);
}

static void wintc_wlan_networks_window_finalize(
    GObject* object
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(object);

    g_free(wnd->connection_name);
    g_free(wnd->pending_ssid);
    g_free(wnd->selected_ssid);

    (G_OBJECT_CLASS(wintc_wlan_networks_window_parent_class))
        ->finalize(object);
}

static void wintc_wlan_networks_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(object);

    switch (prop_id)
    {
        case PROP_DEVICE:
            wnd->device = g_value_dup_object(value);
            break;

        case PROP_CONNECTION_NAME:
            wnd->connection_name = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wlan_networks_window_new(
    WinTCNetDevice* device,
    const gchar*    connection_name
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WLAN_NETWORKS_WINDOW,
            "device",          device,
            "connection-name", connection_name,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_wlan_networks_window_activate_selected(
    WinTCWlanNetworksWindow* wnd
)
{
    if (wnd->dialog_progress)
    {
        return;
    }

    if (!wintc_net_device_get_radio_enabled(wnd->device))
    {
        wintc_net_device_set_enabled_async(
            wnd->device,
            TRUE,
            on_device_action_finished,
            g_object_ref(wnd)
        );

        return;
    }

    WinTCWlanNetwork* network =
        wnd->selected_ssid ?
            wintc_net_device_find_network(wnd->device, wnd->selected_ssid) :
            NULL;

    if (!network)
    {
        return;
    }

    if (network->in_use)
    {
        wintc_wlan_networks_window_disconnect(wnd, network);
    }
    else
    {
        wintc_wlan_networks_window_begin_connect(wnd, network);
    }
}

static void wintc_wlan_networks_window_add_task_group(
    WinTCWlanNetworksWindow* wnd,
    const gchar*             heading,
    const WinTCWlanTaskDef*  tasks,
    gsize                    n_tasks
)
{
    GtkBuilder* builder;
    GtkWidget*  box_group     = NULL;
    GtkWidget*  box_items     = NULL;
    GtkWidget*  label_heading = NULL;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/wlan-tasks.ui"
        );

    wintc_builder_get_objects(
        builder,
        "box-group",     &box_group,
        "box-items",     &box_items,
        "label-heading", &label_heading,
        NULL
    );

    gtk_label_set_text(GTK_LABEL(label_heading), heading);

    gtk_box_pack_start(
        GTK_BOX(wnd->box_tasks),
        box_group,
        FALSE,
        FALSE,
        0
    );

    g_object_unref(builder);

    for (gsize i = 0; i < n_tasks; i++)
    {
        GtkWidget* button_item      = NULL;
        GtkWidget* img_item_icon    = NULL;
        GtkWidget* label_item_label = NULL;

        builder =
            gtk_builder_new_from_resource(
                "/uk/oddmatics/wintc/taskband/wlan-tasks.ui"
            );

        wintc_builder_get_objects(
            builder,
            "button-item",      &button_item,
            "img-item-icon",    &img_item_icon,
            "label-item-label", &label_item_label,
            NULL
        );

        gtk_label_set_text(
            GTK_LABEL(label_item_label),
            _(tasks[i].label)
        );
        wintc_net_image_set_icon_names(
            GTK_IMAGE(img_item_icon),
            tasks[i].icon_names,
            16
        );

        gtk_box_pack_start(
            GTK_BOX(box_items),
            button_item,
            FALSE,
            FALSE,
            0
        );

        if (tasks[i].task != WLAN_TASK_NONE)
        {
            g_object_set_qdata(
                G_OBJECT(button_item),
                S_QUARK_TASK_ID,
                GINT_TO_POINTER(tasks[i].task)
            );

            g_signal_connect(
                button_item,
                "activate-link",
                G_CALLBACK(on_task_activate_link),
                wnd
            );
        }
        else
        {
            // FIXME: No help content to point to yet
            //
            gtk_widget_set_sensitive(button_item, FALSE);
        }

        g_object_unref(builder);
    }
}

static void wintc_wlan_networks_window_begin_connect(
    WinTCWlanNetworksWindow* wnd,
    WinTCWlanNetwork*        network
)
{
    // Take a copy of what we need, the network list may be refreshed while
    // dialogs are open
    //
    g_free(wnd->pending_ssid);

    wnd->pending_ssid        = g_strdup(network->ssid);
    wnd->pending_security    = network->security;
    wnd->pending_has_profile = network->profile_uuid != NULL;

    // Preferred networks have their settings already, just connect
    //
    if (wnd->pending_has_profile)
    {
        wintc_wlan_networks_window_do_connect(wnd, NULL);
        return;
    }

    switch (wnd->pending_security)
    {
        case WINTC_WLAN_SECURITY_NONE:
        {
            GtkWidget* dialog =
                wintc_wlan_unsecured_dialog_new(
                    GTK_WINDOW(wnd),
                    wnd->connection_name,
                    wnd->pending_ssid
                );

            g_signal_connect(
                dialog,
                "response",
                G_CALLBACK(on_dialog_unsecured_response),
                wnd
            );

            gtk_widget_show(dialog);

            break;
        }

        case WINTC_WLAN_SECURITY_OWE:
            wintc_wlan_networks_window_do_connect(wnd, NULL);
            break;

        case WINTC_WLAN_SECURITY_ENTERPRISE:
        {
            gchar* text =
                g_strdup_printf(
                    _("The network '%s' requires 802.1X authentication, which must be set up before connecting. Windows will now open the connection settings."),
                    wnd->pending_ssid
                );

            wintc_messagebox_show(
                GTK_WINDOW(wnd),
                text,
                wnd->connection_name,
                WINTC_BUTTONS_OK,
                WINTC_MESSAGE_INFORMATION
            );

            g_free(text);

            // FIXME: Should be XP's wireless network properties sheet
            //
            wintc_net_launch_tool(
                "nm-connection-editor --create --type=802-11-wireless",
                GTK_WIDGET(wnd)
            );

            break;
        }

        default:
            wintc_wlan_networks_window_show_key_dialog(wnd);
            break;
    }
}

static GtkWidget* wintc_wlan_networks_window_create_row(
    WINTC_UNUSED(WinTCWlanNetworksWindow* wnd),
    WinTCWlanNetwork* network
)
{
    GtkBuilder* builder;
    GtkWidget*  box_signal     = NULL;
    GtkWidget*  box_status     = NULL;
    GtkWidget*  grid_item      = NULL;
    GtkWidget*  img_network    = NULL;
    GtkWidget*  img_security   = NULL;
    GtkWidget*  img_star       = NULL;
    GtkWidget*  label_detail   = NULL;
    GtkWidget*  label_security = NULL;
    GtkWidget*  label_ssid     = NULL;
    GtkWidget*  label_status   = NULL;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/wlan-netitem.ui"
        );

    wintc_builder_get_objects(
        builder,
        "box-signal",     &box_signal,
        "box-status",     &box_status,
        "grid-item",      &grid_item,
        "img-network",    &img_network,
        "img-security",   &img_security,
        "img-star",       &img_star,
        "label-detail",   &label_detail,
        "label-security", &label_security,
        "label-ssid",     &label_ssid,
        "label-status",   &label_status,
        NULL
    );

    // Network name and icon
    //
    gtk_image_set_from_icon_name(
        GTK_IMAGE(img_network),
        "network-wireless",
        GTK_ICON_SIZE_DND
    );
    gtk_label_set_text(GTK_LABEL(label_ssid), network->ssid);

    // Connected / preferred network status, eg. 'Connected' with a star
    //
    if (network->in_use || network->profile_uuid)
    {
        wintc_net_image_set_icon_names(
            GTK_IMAGE(img_star),
            wintc_wlan_get_preferred_icon_names(),
            16
        );

        if (network->in_use)
        {
            gtk_label_set_text(GTK_LABEL(label_status), _("Connected"));
            wintc_widget_add_style_class(label_status, "wintc-wlan-connected");
        }
        else
        {
            gtk_label_set_text(
                GTK_LABEL(label_status),
                network->profile_autoconnect ? _("Automatic") : _("Manual")
            );
        }
    }
    else
    {
        gtk_widget_set_visible(box_status, FALSE);
    }

    // Security
    //
    gchar* security_text = wintc_wlan_security_to_text(network->security);

    gtk_label_set_text(GTK_LABEL(label_security), security_text);

    g_free(security_text);

    if (network->security == WINTC_WLAN_SECURITY_NONE)
    {
        gtk_widget_set_visible(img_security, FALSE);
    }
    else
    {
        wintc_net_image_set_icon_names(
            GTK_IMAGE(img_security),
            wintc_wlan_get_secured_icon_names(),
            16
        );
    }

    // Signal strength
    //
    GtkWidget* img_signal = wintc_wlan_create_signal_image();

    wintc_wlan_signal_image_set_signal(img_signal, network->signal);

    gtk_box_pack_end(GTK_BOX(box_signal), img_signal, FALSE, FALSE, 0);
    gtk_widget_show(img_signal);

    // Extra info, shown when the item is selected - put together from the
    // same sentences as XP
    //
    const gchar* security_detail =
        network->security == WINTC_WLAN_SECURITY_NONE ?
            _("This network is configured for open access. Information sent over this network may be visible to others.") :
            _("This network requires a network key.");

    gchar* detail_text;

    if (network->in_use)
    {
        detail_text =
            g_strdup_printf(
                "%s %s",
                security_detail,
                _("You are currently connected to this network. To disconnect from this network, click Disconnect below.")
            );
    }
    else
    {
        detail_text =
            g_strdup_printf(
                "%s %s",
                security_detail,
                _("If you want to connect to this network, click Connect.")
            );
    }

    gtk_label_set_text(GTK_LABEL(label_detail), detail_text);

    g_free(detail_text);

    // Put it all in a row
    //
    GtkWidget* row = gtk_list_box_row_new();

    gtk_container_add(GTK_CONTAINER(row), grid_item);

    g_object_set_data_full(
        G_OBJECT(row),
        "ssid",
        g_strdup(network->ssid),
        g_free
    );
    g_object_set_data(G_OBJECT(row), "detail", label_detail);

    gtk_widget_show(row);

    g_object_unref(builder);

    return row;
}

static void wintc_wlan_networks_window_disconnect(
    WinTCWlanNetworksWindow* wnd,
    WinTCWlanNetwork*        network
)
{
    gchar* text =
        g_strdup_printf(
            _("Are you sure you wish to disconnect from the network '%s'?\n\nOnce you have disconnected, Windows will not re-connect to this network automatically.\nTo re-connect to this network in the future, select it from the list, and click Connect."),
            network->ssid
        );

    WinTCResponseType response =
        wintc_messagebox_show(
            GTK_WINDOW(wnd),
            text,
            wnd->connection_name,
            WINTC_BUTTONS_YES_NO,
            WINTC_MESSAGE_QUESTION
        );

    g_free(text);

    if (response != WINTC_RESPONSE_YES || wnd->destroyed)
    {
        return;
    }

    wintc_net_device_disconnect_async(
        wnd->device,
        on_device_action_finished,
        g_object_ref(wnd)
    );
}

static void wintc_wlan_networks_window_do_connect(
    WinTCWlanNetworksWindow* wnd,
    const gchar*             key
)
{
    wnd->dialog_progress =
        wintc_wlan_progress_dialog_new(
            GTK_WINDOW(wnd),
            wnd->connection_name,
            wnd->pending_ssid
        );

    g_object_add_weak_pointer(
        G_OBJECT(wnd->dialog_progress),
        (gpointer*) &(wnd->dialog_progress)
    );

    g_signal_connect(
        wnd->dialog_progress,
        "response",
        G_CALLBACK(on_dialog_progress_response),
        wnd
    );

    gtk_widget_show(wnd->dialog_progress);

    g_clear_object(&(wnd->cancellable_connect));
    wnd->cancellable_connect = g_cancellable_new();

    wintc_net_device_connect_wlan_async(
        wnd->device,
        wnd->pending_ssid,
        key,
        wnd->cancellable_connect,
        on_device_connect_finished,
        g_object_ref(wnd)
    );

    wintc_wlan_networks_window_update_button(wnd);
}

static void wintc_wlan_networks_window_show_key_dialog(
    WinTCWlanNetworksWindow* wnd
)
{
    GtkWidget* dialog =
        wintc_wlan_key_dialog_new(
            GTK_WINDOW(wnd),
            wnd->connection_name,
            wnd->pending_ssid,
            wnd->pending_security
        );

    g_signal_connect(
        dialog,
        "response",
        G_CALLBACK(on_dialog_key_response),
        wnd
    );

    gtk_widget_show(dialog);
}

static void wintc_wlan_networks_window_update_button(
    WinTCWlanNetworksWindow* wnd
)
{
    if (!wintc_net_device_get_radio_enabled(wnd->device))
    {
        gtk_button_set_label(GTK_BUTTON(wnd->button_connect), _("_Enable"));
        gtk_widget_set_sensitive(wnd->button_connect, TRUE);
        return;
    }

    WinTCWlanNetwork* network =
        wnd->selected_ssid ?
            wintc_net_device_find_network(wnd->device, wnd->selected_ssid) :
            NULL;

    gtk_button_set_label(
        GTK_BUTTON(wnd->button_connect),
        network && network->in_use ? _("_Disconnect") : _("_Connect")
    );

    gtk_widget_set_sensitive(
        wnd->button_connect,
        network && !(wnd->dialog_progress)
    );
}

static void wintc_wlan_networks_window_update_list(
    WinTCWlanNetworksWindow* wnd
)
{
    WinTCNetState state    = wintc_net_device_get_state(wnd->device);
    GList*        networks = wintc_net_device_get_networks(wnd->device);
    const gchar*  message  = NULL;

    if (!wintc_net_device_get_radio_enabled(wnd->device))
    {
        // FIXME: XP has no equivalent, the connection icon disappears when
        //        the adapter is disabled
        //
        message = _("Wireless networking is turned off. To view the wireless networks in range, click Enable.");
    }
    else if (state == WINTC_NET_STATE_UNAVAILABLE)
    {
        message = _("Windows cannot configure this wireless connection. If you have enabled another program to manage this wireless connection, use that software.");
    }
    else if (!networks && wintc_net_device_is_scanning(wnd->device))
    {
        message = _("Searching for wireless networks in range...");
    }
    else if (!networks)
    {
        message = _("No wireless networks were found in range\n\nMake sure the wireless switch on your computer is on.\nTo see an updated list, click \"Refresh network list.\"");
    }

    if (message)
    {
        gtk_label_set_text(GTK_LABEL(wnd->label_message), message);
        gtk_stack_set_visible_child_name(
            GTK_STACK(wnd->stack_list),
            "message"
        );
    }
    else
    {
        gtk_stack_set_visible_child_name(
            GTK_STACK(wnd->stack_list),
            "list"
        );
    }

    // Rebuild the list, keeping hold of the selection
    //
    GtkListBoxRow* row_to_select = NULL;

    wnd->rebuilding = TRUE;

    wintc_container_clear(GTK_CONTAINER(wnd->list_networks), TRUE);

    for (GList* iter = networks; iter; iter = iter->next)
    {
        WinTCWlanNetwork* network = (WinTCWlanNetwork*) iter->data;
        GtkWidget*        row     =
            wintc_wlan_networks_window_create_row(wnd, network);

        gtk_list_box_insert(GTK_LIST_BOX(wnd->list_networks), row, -1);

        if (g_strcmp0(network->ssid, wnd->selected_ssid) == 0)
        {
            row_to_select = GTK_LIST_BOX_ROW(row);
        }
    }

    if (row_to_select)
    {
        gtk_list_box_select_row(
            GTK_LIST_BOX(wnd->list_networks),
            row_to_select
        );
    }
    else
    {
        g_clear_pointer(&(wnd->selected_ssid), g_free);
    }

    wnd->rebuilding = FALSE;

    wintc_wlan_networks_window_update_button(wnd);
}

//
// CALLBACKS
//
static void on_button_connect_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    wintc_wlan_networks_window_activate_selected(
        WINTC_WLAN_NETWORKS_WINDOW(user_data)
    );
}

static void on_device_action_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    GError* error = NULL;

    if (
        !wintc_net_device_action_finish(
            WINTC_NET_DEVICE(source_object),
            res,
            &error
        )
    )
    {
        if (wnd->destroyed)
        {
            wintc_log_error_and_clear(&error);
        }
        else
        {
            wintc_display_error_and_clear(&error, GTK_WINDOW(wnd));
        }
    }

    g_object_unref(wnd);
}

static void on_device_connect_finished(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCWlanNetworksWindow* wnd    = WINTC_WLAN_NETWORKS_WINDOW(user_data);
    WinTCNetDevice*          device = WINTC_NET_DEVICE(source_object);

    GError*  error = NULL;
    gboolean success;

    success = wintc_net_device_action_finish(device, res, &error);

    g_clear_object(&(wnd->cancellable_connect));

    if (success)
    {
        // XP says 'Connected!' for a moment before closing the dialog
        //
        if (wnd->dialog_progress)
        {
            wintc_wlan_progress_dialog_set_connected(wnd->dialog_progress);

            g_timeout_add(
                WLAN_CONNECTED_LINGER_MS,
                on_timeout_close_progress,
                g_object_ref(wnd)
            );
        }

        g_object_unref(wnd);
        return;
    }

    if (wnd->dialog_progress)
    {
        gtk_widget_destroy(wnd->dialog_progress);
    }

    if (g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
    {
        g_clear_error(&error);
    }
    else if (wnd->destroyed)
    {
        wintc_log_error_and_clear(&error);
    }
    else if (
        g_error_matches(
            error,
            WINTC_NET_ERROR,
            WINTC_NET_ERROR_SECRETS_REQUIRED
        ) &&
        wnd->pending_security != WINTC_WLAN_SECURITY_NONE &&
        wnd->pending_security != WINTC_WLAN_SECURITY_OWE  &&
        wnd->pending_security != WINTC_WLAN_SECURITY_ENTERPRISE
    )
    {
        // Missing or wrong key, ask for it
        //
        WINTC_LOG_DEBUG("wlan: secrets required: %s", error->message);
        g_clear_error(&error);

        wintc_wlan_networks_window_show_key_dialog(wnd);
    }
    else
    {
        WINTC_LOG_DEBUG("wlan: connect failed: %s", error->message);
        g_clear_error(&error);

        wintc_messagebox_show(
            GTK_WINDOW(wnd),
            _("Windows is unable to connect to the selected network. The network may no longer be in range. Please refresh the list of available networks, and try to connect again."),
            wnd->connection_name,
            WINTC_BUTTONS_OK,
            WINTC_MESSAGE_WARNING
        );
    }

    if (!(wnd->destroyed))
    {
        wintc_wlan_networks_window_update_button(wnd);
    }

    g_object_unref(wnd);
}

static void on_device_networks_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    wintc_wlan_networks_window_update_list(
        WINTC_WLAN_NETWORKS_WINDOW(user_data)
    );
}

static void on_device_state_changed(
    WinTCNetDevice* device,
    gpointer        user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    if (wnd->dialog_progress && wnd->cancellable_connect)
    {
        wintc_wlan_progress_dialog_update(wnd->dialog_progress, device);
    }

    wintc_wlan_networks_window_update_button(wnd);
}

static void on_dialog_key_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    if (response_id == GTK_RESPONSE_OK)
    {
        gchar* key =
            g_strdup(wintc_wlan_key_dialog_get_key(GTK_WIDGET(self)));

        gtk_widget_destroy(GTK_WIDGET(self));

        wintc_wlan_networks_window_do_connect(wnd, key);

        explicit_bzero(key, strlen(key));
        g_free(key);

        return;
    }

    gtk_widget_destroy(GTK_WIDGET(self));
}

static void on_dialog_progress_response(
    GtkDialog* self,
    WINTC_UNUSED(gint response_id),
    gpointer   user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    // Any response (Cancel, or closing the dialog) cancels the attempt
    //
    if (wnd->cancellable_connect)
    {
        g_cancellable_cancel(wnd->cancellable_connect);
    }

    gtk_widget_destroy(GTK_WIDGET(self));
}

static void on_dialog_unsecured_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    gtk_widget_destroy(GTK_WIDGET(self));

    if (response_id == GTK_RESPONSE_OK)
    {
        wintc_wlan_networks_window_do_connect(wnd, NULL);
    }
}

static void on_list_networks_row_activated(
    WINTC_UNUSED(GtkListBox*    self),
    WINTC_UNUSED(GtkListBoxRow* row),
    gpointer user_data
)
{
    // Double click / Enter connects, same as XP - the row is already
    // selected by this point
    //
    wintc_wlan_networks_window_activate_selected(
        WINTC_WLAN_NETWORKS_WINDOW(user_data)
    );
}

static void on_list_networks_row_selected(
    GtkListBox*    self,
    GtkListBoxRow* row,
    gpointer       user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    if (wnd->rebuilding && !row)
    {
        return;
    }

    // Only the selected item shows its extra information
    //
    GList* children = gtk_container_get_children(GTK_CONTAINER(self));

    for (GList* iter = children; iter; iter = iter->next)
    {
        GtkWidget* label_detail =
            g_object_get_data(G_OBJECT(iter->data), "detail");

        if (label_detail)
        {
            gtk_widget_set_visible(
                label_detail,
                GTK_LIST_BOX_ROW(iter->data) == row
            );
        }
    }

    g_list_free(children);

    if (!(wnd->rebuilding))
    {
        g_free(wnd->selected_ssid);

        wnd->selected_ssid =
            row ? g_strdup(g_object_get_data(G_OBJECT(row), "ssid")) : NULL;
    }

    wintc_wlan_networks_window_update_button(wnd);
}

static gboolean on_task_activate_link(
    GtkLinkButton* self,
    gpointer       user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    WinTCWlanTask task =
        GPOINTER_TO_INT(
            g_object_get_qdata(G_OBJECT(self), S_QUARK_TASK_ID)
        );

    // FIXME: The setup wizard, preferred networks and advanced settings
    //        should be the XP dialogs, use the network applet's editor
    //        until we have those
    //
    switch (task)
    {
        case WLAN_TASK_REFRESH:
            wintc_net_device_request_scan(wnd->device);
            break;

        case WLAN_TASK_SETUP:
            wintc_net_launch_tool(
                "nm-connection-editor --create --type=802-11-wireless",
                GTK_WIDGET(wnd)
            );
            break;

        case WLAN_TASK_PREFERRED_ORDER:
            wintc_net_launch_tool(
                "nm-connection-editor",
                GTK_WIDGET(wnd)
            );
            break;

        case WLAN_TASK_ADVANCED:
        {
            const gchar* uuid =
                wintc_net_device_get_connection_uuid(wnd->device);

            if (uuid)
            {
                gchar* cmdline =
                    g_strdup_printf("nm-connection-editor --edit=%s", uuid);

                wintc_net_launch_tool(cmdline, GTK_WIDGET(wnd));

                g_free(cmdline);
            }
            else
            {
                wintc_net_launch_tool(
                    "nm-connection-editor",
                    GTK_WIDGET(wnd)
                );
            }

            break;
        }

        default:
            break;
    }

    return TRUE;
}

static gboolean on_timeout_close_progress(
    gpointer user_data
)
{
    WinTCWlanNetworksWindow* wnd = WINTC_WLAN_NETWORKS_WINDOW(user_data);

    if (wnd->dialog_progress)
    {
        gtk_widget_destroy(wnd->dialog_progress);
    }

    if (!(wnd->destroyed))
    {
        wintc_wlan_networks_window_update_button(wnd);
    }

    g_object_unref(wnd);

    return G_SOURCE_REMOVE;
}

static void on_window_destroy(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_WLAN_NETWORKS_WINDOW(self)->destroyed = TRUE;
}
