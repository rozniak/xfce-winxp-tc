#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "device.h"
#include "dialogs.h"
#include "statwnd.h"
#include "util.h"

//
// PRIVATE CONSTANTS
//
#define NET_STATUS_UPDATE_INTERVAL_S 1

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

enum
{
    SIGNAL_VIEW_NETWORKS = 0,
    N_SIGNALS
};

enum
{
    DETAILS_COLUMN_PROPERTY = 0,
    DETAILS_COLUMN_VALUE,
    N_DETAILS_COLUMNS
};

//
// FORWARD DECLARATIONS
//
static void wintc_net_status_window_constructed(
    GObject* object
);
static void wintc_net_status_window_dispose(
    GObject* object
);
static void wintc_net_status_window_finalize(
    GObject* object
);
static void wintc_net_status_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_net_status_window_show_details(
    WinTCNetStatusWindow* wnd
);
static void wintc_net_status_window_update(
    WinTCNetStatusWindow* wnd
);
static void wintc_net_status_window_update_activity(
    WinTCNetStatusWindow* wnd
);
static void wintc_net_status_window_update_traffic(
    WinTCNetStatusWindow* wnd
);

static gchar* net_format_lease_time(
    gint64 unix_time
);

static void on_button_close_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_details_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_disable_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_properties_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_repair_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_view_networks_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_device_activity_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_device_state_changed(
    WinTCNetDevice* device,
    gpointer        user_data
);
static void on_dialog_details_response(
    GtkDialog* self,
    gint       response_id,
    gpointer   user_data
);
static gboolean on_timeout_update(
    gpointer user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_net_status_window_properties[N_PROPERTIES] = {
    0
};
static guint wintc_net_status_window_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNetStatusWindow
{
    GtkWindow __parent__;

    WinTCNetDevice* device;
    gchar*          connection_name;

    gulong sigid_activity_changed;
    gulong sigid_state_changed;
    guint  id_update;

    // UI
    //
    GtkWidget* button_disable;
    GtkWidget* button_properties;
    GtkWidget* button_repair;
    GtkWidget* button_view_networks;
    GtkWidget* img_activity;
    GtkWidget* img_signal;
    GtkWidget* img_support;
    GtkWidget* label_address_type;
    GtkWidget* label_duration;
    GtkWidget* label_gateway;
    GtkWidget* label_ip_address;
    GtkWidget* label_network;
    GtkWidget* label_network_caption;
    GtkWidget* label_received;
    GtkWidget* label_repair;
    GtkWidget* label_sent;
    GtkWidget* label_signal_caption;
    GtkWidget* label_speed;
    GtkWidget* label_status;
    GtkWidget* label_subnet_mask;

    GtkWidget* dialog_details;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNetStatusWindow,
    wintc_net_status_window,
    GTK_TYPE_WINDOW
)

static void wintc_net_status_window_class_init(
    WinTCNetStatusWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_net_status_window_constructed;
    object_class->dispose      = wintc_net_status_window_dispose;
    object_class->finalize     = wintc_net_status_window_finalize;
    object_class->set_property = wintc_net_status_window_set_property;

    wintc_net_status_window_properties[PROP_DEVICE] =
        g_param_spec_object(
            "device",
            "Device",
            "The network adapter.",
            WINTC_TYPE_NET_DEVICE,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );
    wintc_net_status_window_properties[PROP_CONNECTION_NAME] =
        g_param_spec_string(
            "connection-name",
            "ConnectionName",
            "The display name of the network connection.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_net_status_window_properties
    );

    wintc_net_status_window_signals[SIGNAL_VIEW_NETWORKS] =
        g_signal_new(
            "view-networks",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_net_status_window_init(
    WinTCNetStatusWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  box_signal     = NULL;
    GtkWidget*  button_close   = NULL;
    GtkWidget*  button_details = NULL;
    GtkWidget*  main_box       = NULL;

    wintc_net_ensure_styles();

    gtk_window_set_position(GTK_WINDOW(self), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(self), FALSE);

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-net-status");

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/net-statwnd.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-box",              &main_box,
        "box-signal",            &box_signal,
        "button-close",          &button_close,
        "button-details",        &button_details,
        "button-disable",        &(self->button_disable),
        "button-properties",     &(self->button_properties),
        "button-repair",         &(self->button_repair),
        "button-view-networks",  &(self->button_view_networks),
        "img-activity",          &(self->img_activity),
        "img-support",           &(self->img_support),
        "label-address-type",    &(self->label_address_type),
        "label-duration",        &(self->label_duration),
        "label-gateway",         &(self->label_gateway),
        "label-ip-address",      &(self->label_ip_address),
        "label-network",         &(self->label_network),
        "label-network-caption", &(self->label_network_caption),
        "label-received",        &(self->label_received),
        "label-repair",          &(self->label_repair),
        "label-sent",            &(self->label_sent),
        "label-signal-caption",  &(self->label_signal_caption),
        "label-speed",           &(self->label_speed),
        "label-status",          &(self->label_status),
        "label-subnet-mask",     &(self->label_subnet_mask),
        NULL
    );

    gtk_container_add(GTK_CONTAINER(self), main_box);

    self->img_signal = wintc_wlan_create_signal_image();

    gtk_box_pack_end(GTK_BOX(box_signal), self->img_signal, FALSE, FALSE, 0);
    gtk_widget_show(self->img_signal);

    g_object_unref(builder);

    // Signals
    //
    g_signal_connect(
        button_close,
        "clicked",
        G_CALLBACK(on_button_close_clicked),
        self
    );
    g_signal_connect(
        button_details,
        "clicked",
        G_CALLBACK(on_button_details_clicked),
        self
    );
    g_signal_connect(
        self->button_disable,
        "clicked",
        G_CALLBACK(on_button_disable_clicked),
        self
    );
    g_signal_connect(
        self->button_properties,
        "clicked",
        G_CALLBACK(on_button_properties_clicked),
        self
    );
    g_signal_connect(
        self->button_repair,
        "clicked",
        G_CALLBACK(on_button_repair_clicked),
        self
    );
    g_signal_connect(
        self->button_view_networks,
        "clicked",
        G_CALLBACK(on_button_view_networks_clicked),
        self
    );

    gtk_window_set_default(GTK_WINDOW(self), button_close);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_net_status_window_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_net_status_window_parent_class))
        ->constructed(object);

    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(object);

    gboolean wireless =
        wintc_net_device_get_kind(wnd->device) ==
            WINTC_NET_DEVICE_KIND_WIRELESS;

    gchar* title = g_strdup_printf(_("%s Status"), wnd->connection_name);

    gtk_window_set_title(GTK_WINDOW(wnd), title);

    g_free(title);

    gtk_window_set_icon_name(
        GTK_WINDOW(wnd),
        wireless ? "network-wireless" : "network-wired"
    );
    gtk_image_set_from_icon_name(
        GTK_IMAGE(wnd->img_support),
        wireless ? "network-wireless" : "network-wired",
        GTK_ICON_SIZE_DND
    );

    // The wireless sheet has the network name, signal strength and a way
    // to the network list
    //
    gtk_widget_set_visible(wnd->label_network,         wireless);
    gtk_widget_set_visible(wnd->label_network_caption, wireless);
    gtk_widget_set_visible(wnd->label_signal_caption,  wireless);
    gtk_widget_set_visible(wnd->img_signal,            wireless);
    gtk_widget_set_visible(wnd->button_view_networks,  wireless);

    wnd->sigid_state_changed =
        g_signal_connect(
            wnd->device,
            "state-changed",
            G_CALLBACK(on_device_state_changed),
            wnd
        );
    wnd->sigid_activity_changed =
        g_signal_connect(
            wnd->device,
            "activity-changed",
            G_CALLBACK(on_device_activity_changed),
            wnd
        );

    // Duration and byte counts tick along while the window is open
    //
    wnd->id_update =
        g_timeout_add_seconds(
            NET_STATUS_UPDATE_INTERVAL_S,
            on_timeout_update,
            wnd
        );

    wintc_net_status_window_update(wnd);
}

static void wintc_net_status_window_dispose(
    GObject* object
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(object);

    g_clear_handle_id(&(wnd->id_update), g_source_remove);

    if (wnd->device)
    {
        g_clear_signal_handler(&(wnd->sigid_state_changed),    wnd->device);
        g_clear_signal_handler(&(wnd->sigid_activity_changed), wnd->device);
    }

    g_clear_object(&(wnd->device));

    (G_OBJECT_CLASS(wintc_net_status_window_parent_class))
        ->dispose(object);
}

static void wintc_net_status_window_finalize(
    GObject* object
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(object);

    g_free(wnd->connection_name);

    (G_OBJECT_CLASS(wintc_net_status_window_parent_class))
        ->finalize(object);
}

static void wintc_net_status_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(object);

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
GtkWidget* wintc_net_status_window_new(
    WinTCNetDevice* device,
    const gchar*    connection_name
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NET_STATUS_WINDOW,
            "device",          device,
            "connection-name", connection_name,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_net_status_window_show_details(
    WinTCNetStatusWindow* wnd
)
{
    if (wnd->dialog_details)
    {
        gtk_window_present(GTK_WINDOW(wnd->dialog_details));
        return;
    }

    WinTCNetIpInfo* info = wintc_net_device_get_ip_info(wnd->device);

    // Build the property list
    //
    GtkListStore* store =
        gtk_list_store_new(
            N_DETAILS_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_STRING
        );

    gchar* hwaddr   = g_strdup(info->hwaddr ? info->hwaddr : "");
    gchar* mask     = info->address ?
                          wintc_net_prefix_to_netmask(info->prefix) :
                          g_strdup("");
    gchar* obtained = NULL;
    gchar* expires  = NULL;

    // XP writes MAC addresses with dashes
    //
    g_strdelimit(hwaddr, ":", '-');

    if (info->dhcp && info->lease_expires)
    {
        expires = net_format_lease_time(info->lease_expires);

        if (info->lease_obtained)
        {
            obtained = net_format_lease_time(info->lease_obtained);
        }
    }

    struct
    {
        const gchar* property;
        const gchar* value;
    } rows[] = {
        { _("Physical Address"), hwaddr                                   },
        { _("IP Address"),       info->address ? info->address : ""       },
        { _("Subnet Mask"),      mask                                     },
        { _("Default Gateway"),  info->gateway ? info->gateway : ""       },
        { _("DHCP Server"),      info->dhcp_server ? info->dhcp_server : "" },
        { _("Lease Obtained"),   obtained ? obtained : ""                 },
        { _("Lease Expires"),    expires  ? expires  : ""                 }
    };

    for (gsize i = 0; i < G_N_ELEMENTS(rows); i++)
    {
        gtk_list_store_insert_with_values(
            store,
            NULL,
            -1,
            DETAILS_COLUMN_PROPERTY, rows[i].property,
            DETAILS_COLUMN_VALUE,    rows[i].value,
            -1
        );
    }

    // DNS servers, the first goes on the same line as the property
    //
    if (info->dns && info->dns[0])
    {
        for (gint i = 0; info->dns[i]; i++)
        {
            gtk_list_store_insert_with_values(
                store,
                NULL,
                -1,
                DETAILS_COLUMN_PROPERTY, i == 0 ? _("DNS Server") : "",
                DETAILS_COLUMN_VALUE,    info->dns[i],
                -1
            );
        }
    }
    else
    {
        gtk_list_store_insert_with_values(
            store,
            NULL,
            -1,
            DETAILS_COLUMN_PROPERTY, _("DNS Server"),
            DETAILS_COLUMN_VALUE,    "",
            -1
        );
    }

    gtk_list_store_insert_with_values(
        store,
        NULL,
        -1,
        DETAILS_COLUMN_PROPERTY, _("WINS Server"),
        DETAILS_COLUMN_VALUE,    "",
        -1
    );

    g_free(hwaddr);
    g_free(mask);
    g_free(obtained);
    g_free(expires);

    wintc_net_ip_info_free(info);

    // Build the dialog
    //
    GtkWidget* tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    g_object_unref(store);

    gtk_tree_view_insert_column_with_attributes(
        GTK_TREE_VIEW(tree_view),
        -1,
        _("Property"),
        gtk_cell_renderer_text_new(),
        "text", DETAILS_COLUMN_PROPERTY,
        NULL
    );
    gtk_tree_view_insert_column_with_attributes(
        GTK_TREE_VIEW(tree_view),
        -1,
        _("Value"),
        gtk_cell_renderer_text_new(),
        "text", DETAILS_COLUMN_VALUE,
        NULL
    );

    GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);

    gtk_scrolled_window_set_shadow_type(
        GTK_SCROLLED_WINDOW(scrolled),
        GTK_SHADOW_IN
    );
    gtk_widget_set_size_request(scrolled, 320, 240);
    gtk_container_add(GTK_CONTAINER(scrolled), tree_view);

    wnd->dialog_details =
        gtk_dialog_new_with_buttons(
            _("Network Connection Details"),
            GTK_WINDOW(wnd),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            wintc_lc_get_control_text(WINTC_CTLTXT_CLOSE, WINTC_PUNC_NONE),
            GTK_RESPONSE_CLOSE,
            NULL
        );

    g_object_add_weak_pointer(
        G_OBJECT(wnd->dialog_details),
        (gpointer*) &(wnd->dialog_details)
    );

    gtk_window_set_icon_name(
        GTK_WINDOW(wnd->dialog_details),
        gtk_window_get_icon_name(GTK_WINDOW(wnd))
    );
    gtk_window_set_skip_taskbar_hint(
        GTK_WINDOW(wnd->dialog_details),
        TRUE
    );

    GtkWidget* box_content =
        gtk_dialog_get_content_area(GTK_DIALOG(wnd->dialog_details));
    GtkWidget* label_heading =
        gtk_label_new(_("Network Connection Details:"));

    gtk_label_set_xalign(GTK_LABEL(label_heading), 0.0f);

    wintc_widget_add_style_class(label_heading, "wintc-mb-sm");
    wintc_widget_add_style_class(scrolled,      "wintc-mb-md");
    wintc_widget_add_style_class(box_content,   "wintc-ml-md");
    wintc_widget_add_style_class(box_content,   "wintc-mr-md");
    wintc_widget_add_style_class(box_content,   "wintc-mt-md");

    gtk_box_pack_start(GTK_BOX(box_content), label_heading, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box_content), scrolled,      TRUE,  TRUE,  0);

    g_signal_connect(
        wnd->dialog_details,
        "response",
        G_CALLBACK(on_dialog_details_response),
        NULL
    );

    gtk_widget_show_all(wnd->dialog_details);
}

static void wintc_net_status_window_update(
    WinTCNetStatusWindow* wnd
)
{
    WinTCNetState state     = wintc_net_device_get_state(wnd->device);
    gboolean      connected = wintc_net_device_is_connected(wnd->device);
    gchar*        text;

    // General
    //
    gtk_label_set_text(
        GTK_LABEL(wnd->label_status),
        wintc_net_state_to_text(
            wintc_net_device_get_kind(wnd->device),
            state
        )
    );

    if (
        wintc_net_device_get_kind(wnd->device) ==
            WINTC_NET_DEVICE_KIND_WIRELESS
    )
    {
        WinTCWlanNetwork* active =
            wintc_net_device_get_active_network(wnd->device);

        gtk_label_set_text(
            GTK_LABEL(wnd->label_network),
            active ? active->ssid : ""
        );

        if (active)
        {
            wintc_wlan_signal_image_set_signal(wnd->img_signal, active->signal);
        }

        gtk_widget_set_visible(wnd->img_signal, active != NULL);
    }

    text =
        connected ?
            wintc_net_speed_to_text(wintc_net_device_get_speed(wnd->device)) :
            g_strdup("");

    gtk_label_set_text(GTK_LABEL(wnd->label_speed), text);
    g_free(text);

    wintc_net_status_window_update_traffic(wnd);
    wintc_net_status_window_update_activity(wnd);

    // Support
    //
    WinTCNetIpInfo* info = wintc_net_device_get_ip_info(wnd->device);

    if (info->address)
    {
        const gchar* address_type = _("Manually Configured");

        if (g_str_has_prefix(info->address, "169.254."))
        {
            address_type = _("Automatic Private Address");
        }
        else if (info->dhcp)
        {
            address_type = _("Assigned by DHCP");
        }

        gtk_label_set_text(GTK_LABEL(wnd->label_address_type), address_type);
        gtk_label_set_text(GTK_LABEL(wnd->label_ip_address),   info->address);

        text = wintc_net_prefix_to_netmask(info->prefix);
        gtk_label_set_text(GTK_LABEL(wnd->label_subnet_mask), text);
        g_free(text);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(wnd->label_address_type), "");
        gtk_label_set_text(GTK_LABEL(wnd->label_ip_address),   "");
        gtk_label_set_text(GTK_LABEL(wnd->label_subnet_mask),  "");
    }

    gtk_label_set_text(
        GTK_LABEL(wnd->label_gateway),
        info->gateway ? info->gateway : ""
    );

    wintc_net_ip_info_free(info);

    gtk_label_set_text(
        GTK_LABEL(wnd->label_repair),
        state == WINTC_NET_STATE_LIMITED ?
            _("To restart the connection and try to establish full connectivity, click Repair.") :
            _("Windows did not detect problems with this connection. If you cannot connect, click Repair.")
    );

    // Buttons
    //
    gtk_widget_set_sensitive(
        wnd->button_properties,
        wintc_net_device_get_connection_uuid(wnd->device) != NULL
    );
    gtk_widget_set_sensitive(
        wnd->button_disable,
        state != WINTC_NET_STATE_DISABLED
    );
    gtk_widget_set_sensitive(
        wnd->button_repair,
        connected
    );
}

static void wintc_net_status_window_update_activity(
    WinTCNetStatusWindow* wnd
)
{
    wintc_net_image_set_icon_names(
        GTK_IMAGE(wnd->img_activity),
        wintc_net_get_activity_icon_names(
            wintc_net_device_get_activity(wnd->device)
        ),
        32
    );
}

static void wintc_net_status_window_update_traffic(
    WinTCNetStatusWindow* wnd
)
{
    gint64 since = wintc_net_device_get_connected_since(wnd->device);
    gchar* text;

    if (wintc_net_device_is_connected(wnd->device) && since)
    {
        gint64 secs = (g_get_real_time() / G_USEC_PER_SEC) - since;

        if (secs < 0)
        {
            secs = 0;
        }

        // XP adds the days in front, eg. 1 day 02:03:04
        //
        gint64 days = secs / 86400;

        gchar* hms =
            g_strdup_printf(
                "%02d:%02d:%02d",
                (gint) ((secs / 3600) % 24),
                (gint) ((secs / 60) % 60),
                (gint) (secs % 60)
            );

        if (days)
        {
            text =
                g_strdup_printf(
                    "%" G_GINT64_FORMAT "%s %s",
                    days,
                    ngettext(" day", " days", days),
                    hms
                );

            g_free(hms);
        }
        else
        {
            text = hms;
        }
    }
    else
    {
        text = g_strdup("");
    }

    gtk_label_set_text(GTK_LABEL(wnd->label_duration), text);
    g_free(text);

    // XP shows packets and groups the digits, eg. 1,234 - use the locale's
    // grouping
    //
    WinTCNetTraffic traffic;

    wintc_net_device_get_traffic(wnd->device, &traffic);

    text = g_strdup_printf("%'" G_GUINT64_FORMAT, traffic.tx_packets);
    gtk_label_set_text(GTK_LABEL(wnd->label_sent), text);
    g_free(text);

    text = g_strdup_printf("%'" G_GUINT64_FORMAT, traffic.rx_packets);
    gtk_label_set_text(GTK_LABEL(wnd->label_received), text);
    g_free(text);
}

static gchar* net_format_lease_time(
    gint64 unix_time
)
{
    GDateTime* date_time = g_date_time_new_from_unix_local(unix_time);

    if (!date_time)
    {
        return g_strdup("");
    }

    gchar* text = g_date_time_format(date_time, "%x %X");

    g_date_time_unref(date_time);

    return text;
}

//
// CALLBACKS
//
static void on_button_close_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

static void on_button_details_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    wintc_net_status_window_show_details(
        WINTC_NET_STATUS_WINDOW(user_data)
    );
}

static void on_button_disable_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(user_data);

    wintc_net_device_set_enabled_async(wnd->device, FALSE, NULL, NULL);

    gtk_window_close(GTK_WINDOW(wnd));
}

static void on_button_properties_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(user_data);

    const gchar* uuid = wintc_net_device_get_connection_uuid(wnd->device);

    if (!uuid)
    {
        return;
    }

    // FIXME: Should be the XP connection properties sheet, use the network
    //        applet's editor until we have one
    //
    gchar* cmdline = g_strdup_printf("nm-connection-editor --edit=%s", uuid);

    wintc_net_launch_tool(cmdline, GTK_WIDGET(wnd));

    g_free(cmdline);
}

static void on_button_repair_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCNetStatusWindow* wnd = WINTC_NET_STATUS_WINDOW(user_data);

    wintc_net_repair_connection(
        wnd->device,
        wnd->connection_name,
        GTK_WIDGET(wnd)
    );
}

static void on_button_view_networks_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    g_signal_emit(
        user_data,
        wintc_net_status_window_signals[SIGNAL_VIEW_NETWORKS],
        0
    );
}

static void on_device_activity_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    wintc_net_status_window_update_activity(
        WINTC_NET_STATUS_WINDOW(user_data)
    );
}

static void on_device_state_changed(
    WINTC_UNUSED(WinTCNetDevice* device),
    gpointer user_data
)
{
    wintc_net_status_window_update(WINTC_NET_STATUS_WINDOW(user_data));
}

static void on_dialog_details_response(
    GtkDialog* self,
    WINTC_UNUSED(gint response_id),
    WINTC_UNUSED(gpointer user_data)
)
{
    gtk_widget_destroy(GTK_WIDGET(self));
}

static gboolean on_timeout_update(
    gpointer user_data
)
{
    wintc_net_status_window_update_traffic(
        WINTC_NET_STATUS_WINDOW(user_data)
    );

    return G_SOURCE_CONTINUE;
}
