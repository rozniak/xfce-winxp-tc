#include <glib.h>
#include <glib/gi18n.h>
#include <NetworkManager.h>
#include <string.h>
#include <wintc/comgtk.h>

#include "../device.h"
#include "../util.h"
#include "backend.h"

//
// PRIVATE CONSTANTS
//
#define NET_ACTIVITY_INTERVAL_MS 500
#define NET_UPDATE_DELAY_MS      100

#define NET_CONNECT_TIMEOUT_S 60
#define NET_REPAIR_TIMEOUT_S  45
#define NET_SCAN_TIMEOUT_S    15

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_NM_CLIENT,
    PROP_NM_DEVICE,
    N_PROPERTIES
};

enum
{
    SIGNAL_STATE_CHANGED = 0,
    SIGNAL_NETWORKS_CHANGED,
    SIGNAL_ACTIVITY_CHANGED,
    N_SIGNALS
};

//
// PRIVATE STRUCTURES
//

// Data for an action that waits on NetworkManager to activate a connection
//
typedef struct _WinTCNetActionData
{
    NMRemoteConnection* connection;
    gchar*              ap_path;
    guint               timeout_s;

    NMActiveConnection* active_connection;
    GCancellable*       cancellable;
    gulong              sigid_ac_state;
    gulong              sigid_cancelled;
    guint               id_timeout;
    guint               id_cancel;
    gboolean            completed;
} WinTCNetActionData;

//
// FORWARD DECLARATIONS
//
static void wintc_net_device_constructed(
    GObject* object
);
static void wintc_net_device_dispose(
    GObject* object
);
static void wintc_net_device_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_net_device_queue_update(
    WinTCNetDevice* device,
    gboolean        networks
);
static void wintc_net_device_update_networks(
    WinTCNetDevice* device
);
static void wintc_net_device_update_state(
    WinTCNetDevice* device
);
static void wintc_net_device_update_traffic(
    WinTCNetDevice* device
);
static void wintc_net_device_set_traffic_polling(
    WinTCNetDevice* device,
    gboolean        enabled
);

static void net_action_complete(
    GTask*  task,
    GError* error
);
static void net_action_data_free(
    WinTCNetActionData* data
);
static void net_action_watch(
    GTask*              task,
    NMActiveConnection* active_connection
);
static void net_activate_connection(
    GTask* task
);
static NMConnection* net_build_wlan_connection(
    WinTCWlanNetwork* network,
    const gchar*      key
);
static NMRemoteConnection* net_find_wlan_profile(
    WinTCNetDevice* device,
    GBytes*         ssid
);
static GError* net_error_from_reason(
    NMActiveConnectionStateReason reason
);
static WinTCWlanSecurity net_get_ap_security(
    NMAccessPoint* ap
);
static gboolean net_ip_config_has_routable_address(
    NMIPConfig* ip_config
);
static gboolean net_read_traffic(
    const gchar*     iface,
    WinTCNetTraffic* traffic
);
static void net_set_wlan_key(
    NMConnection*     connection,
    WinTCWlanSecurity security,
    const gchar*      key
);

static void on_action_cancelled(
    GCancellable* cancellable,
    gpointer      user_data
);
static gboolean on_action_cancelled_idle(
    gpointer user_data
);
static gboolean on_action_timeout(
    gpointer user_data
);
static void on_active_connection_state_changed(
    NMActiveConnection* active_connection,
    guint               state,
    guint               reason,
    gpointer            user_data
);
static void on_nm_client_activate_connection_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_client_add_and_activate_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_client_connections_changed(
    NMClient* nm_client,
    GObject*  connection,
    gpointer  user_data
);
static void on_nm_client_notify(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_nm_client_set_property_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_device_access_points_changed(
    NMDeviceWifi* nm_device,
    GObject*      ap,
    gpointer      user_data
);
static void on_nm_device_disconnect_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_device_notify(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_nm_device_request_scan_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_remote_connection_update2_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static gboolean on_timeout_activity(
    gpointer user_data
);
static gboolean on_timeout_scan(
    gpointer user_data
);
static gboolean on_timeout_update(
    gpointer user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_net_device_properties[N_PROPERTIES] = { 0 };
static guint       wintc_net_device_signals[N_SIGNALS]       = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNetDevice
{
    GObject __parent__;

    NMClient*          nm_client;
    NMDevice*          nm_device;
    WinTCNetDeviceKind kind;

    // State
    //
    WinTCNetState state;
    gint64        connected_since;
    gboolean      state_known;

    guint    id_update;
    gboolean update_networks;

    // Wireless
    //
    GList*   networks;
    gboolean scanning;
    guint    id_scan_timeout;

    // Traffic
    //
    WinTCNetTraffic  traffic;
    gboolean         traffic_valid;
    WinTCNetActivity activity;
    guint            id_activity;

    // Signal handlers
    //
    gulong sigid_client_notify;
    gulong sigid_connection_added;
    gulong sigid_connection_removed;
    gulong sigid_device_notify;
    gulong sigid_ap_added;
    gulong sigid_ap_removed;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNetDevice,
    wintc_net_device,
    G_TYPE_OBJECT
)

static void wintc_net_device_class_init(
    WinTCNetDeviceClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_net_device_constructed;
    object_class->dispose      = wintc_net_device_dispose;
    object_class->set_property = wintc_net_device_set_property;

    wintc_net_device_properties[PROP_NM_CLIENT] =
        g_param_spec_object(
            "nm-client",
            "NMClient",
            "The NetworkManager client.",
            NM_TYPE_CLIENT,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );
    wintc_net_device_properties[PROP_NM_DEVICE] =
        g_param_spec_object(
            "nm-device",
            "NMDevice",
            "The NetworkManager device.",
            NM_TYPE_DEVICE,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_net_device_properties
    );

    wintc_net_device_signals[SIGNAL_STATE_CHANGED] =
        g_signal_new(
            "state-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED] =
        g_signal_new(
            "networks-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_net_device_signals[SIGNAL_ACTIVITY_CHANGED] =
        g_signal_new(
            "activity-changed",
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

static void wintc_net_device_init(
    WINTC_UNUSED(WinTCNetDevice* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_net_device_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_net_device_parent_class))->constructed(object);

    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    device->kind =
        NM_IS_DEVICE_WIFI(device->nm_device) ?
            WINTC_NET_DEVICE_KIND_WIRELESS :
            WINTC_NET_DEVICE_KIND_WIRED;

    // Any property change on the device may affect what we show, updates
    // are batched up so that a burst of changes is handled once
    //
    device->sigid_device_notify =
        g_signal_connect(
            device->nm_device,
            "notify",
            G_CALLBACK(on_nm_device_notify),
            device
        );
    device->sigid_client_notify =
        g_signal_connect(
            device->nm_client,
            "notify",
            G_CALLBACK(on_nm_client_notify),
            device
        );

    if (device->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        device->sigid_ap_added =
            g_signal_connect(
                device->nm_device,
                "access-point-added",
                G_CALLBACK(on_nm_device_access_points_changed),
                device
            );
        device->sigid_ap_removed =
            g_signal_connect(
                device->nm_device,
                "access-point-removed",
                G_CALLBACK(on_nm_device_access_points_changed),
                device
            );

        // Preferred networks are just the saved profiles
        //
        device->sigid_connection_added =
            g_signal_connect(
                device->nm_client,
                "connection-added",
                G_CALLBACK(on_nm_client_connections_changed),
                device
            );
        device->sigid_connection_removed =
            g_signal_connect(
                device->nm_client,
                "connection-removed",
                G_CALLBACK(on_nm_client_connections_changed),
                device
            );

        wintc_net_device_update_networks(device);
    }

    wintc_net_device_update_state(device);
}

static void wintc_net_device_dispose(
    GObject* object
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    g_clear_handle_id(&(device->id_update),       g_source_remove);
    g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);
    g_clear_handle_id(&(device->id_activity),     g_source_remove);

    if (device->nm_device)
    {
        g_clear_signal_handler(
            &(device->sigid_device_notify),
            device->nm_device
        );
        g_clear_signal_handler(
            &(device->sigid_ap_added),
            device->nm_device
        );
        g_clear_signal_handler(
            &(device->sigid_ap_removed),
            device->nm_device
        );
    }

    if (device->nm_client)
    {
        g_clear_signal_handler(
            &(device->sigid_client_notify),
            device->nm_client
        );
        g_clear_signal_handler(
            &(device->sigid_connection_added),
            device->nm_client
        );
        g_clear_signal_handler(
            &(device->sigid_connection_removed),
            device->nm_client
        );
    }

    g_list_free_full(
        g_steal_pointer(&(device->networks)),
        (GDestroyNotify) wintc_wlan_network_free
    );

    g_clear_object(&(device->nm_device));
    g_clear_object(&(device->nm_client));

    (G_OBJECT_CLASS(wintc_net_device_parent_class))->dispose(object);
}

static void wintc_net_device_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    switch (prop_id)
    {
        case PROP_NM_CLIENT:
            device->nm_client = g_value_dup_object(value);
            break;

        case PROP_NM_DEVICE:
            device->nm_device = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCNetDevice* wintc_net_device_new(
    NMClient* nm_client,
    NMDevice* nm_device
)
{
    return WINTC_NET_DEVICE(
        g_object_new(
            WINTC_TYPE_NET_DEVICE,
            "nm-client", nm_client,
            "nm-device", nm_device,
            NULL
        )
    );
}

NMDevice* wintc_net_device_get_nm_device(
    WinTCNetDevice* device
)
{
    return device->nm_device;
}

WinTCNetDeviceKind wintc_net_device_get_kind(
    WinTCNetDevice* device
)
{
    return device->kind;
}

const gchar* wintc_net_device_get_interface(
    WinTCNetDevice* device
)
{
    return nm_device_get_iface(device->nm_device);
}

WinTCNetState wintc_net_device_get_state(
    WinTCNetDevice* device
)
{
    return device->state;
}

gboolean wintc_net_device_is_connected(
    WinTCNetDevice* device
)
{
    return device->state == WINTC_NET_STATE_CONNECTED ||
           device->state == WINTC_NET_STATE_LIMITED;
}

gint64 wintc_net_device_get_connected_since(
    WinTCNetDevice* device
)
{
    return device->connected_since;
}

guint32 wintc_net_device_get_speed(
    WinTCNetDevice* device
)
{
    // Speed in Kbps
    //
    if (NM_IS_DEVICE_WIFI(device->nm_device))
    {
        return nm_device_wifi_get_bitrate(NM_DEVICE_WIFI(device->nm_device));
    }

    if (NM_IS_DEVICE_ETHERNET(device->nm_device))
    {
        return nm_device_ethernet_get_speed(
            NM_DEVICE_ETHERNET(device->nm_device)
        ) * 1000;
    }

    return 0;
}

const gchar* wintc_net_device_get_connection_uuid(
    WinTCNetDevice* device
)
{
    NMActiveConnection* active_connection =
        nm_device_get_active_connection(device->nm_device);

    if (!active_connection)
    {
        return NULL;
    }

    return nm_active_connection_get_uuid(active_connection);
}

WinTCNetIpInfo* wintc_net_device_get_ip_info(
    WinTCNetDevice* device
)
{
    WinTCNetIpInfo* info = g_new0(WinTCNetIpInfo, 1);

    info->hwaddr = g_strdup(nm_device_get_hw_address(device->nm_device));

    // Only report addresses once the connection is up, during activation the
    // config can be half populated
    //
    NMDeviceState nm_state = nm_device_get_state(device->nm_device);

    if (
        nm_state < NM_DEVICE_STATE_IP_CONFIG ||
        nm_state > NM_DEVICE_STATE_ACTIVATED
    )
    {
        return info;
    }

    NMIPConfig* ip4_config = nm_device_get_ip4_config(device->nm_device);

    if (ip4_config)
    {
        GPtrArray* addresses = nm_ip_config_get_addresses(ip4_config);

        if (addresses && addresses->len)
        {
            NMIPAddress* address = g_ptr_array_index(addresses, 0);

            info->address = g_strdup(nm_ip_address_get_address(address));
            info->prefix  = nm_ip_address_get_prefix(address);
        }

        info->gateway = g_strdup(nm_ip_config_get_gateway(ip4_config));
        info->dns     =
            g_strdupv(
                (gchar**) nm_ip_config_get_nameservers(ip4_config)
            );
    }

    NMDhcpConfig* dhcp4_config =
        nm_device_get_dhcp4_config(device->nm_device);

    if (dhcp4_config)
    {
        const gchar* expiry     =
            nm_dhcp_config_get_one_option(dhcp4_config, "expiry");
        const gchar* lease_time =
            nm_dhcp_config_get_one_option(dhcp4_config, "dhcp_lease_time");

        info->dhcp        = TRUE;
        info->dhcp_server =
            g_strdup(
                nm_dhcp_config_get_one_option(
                    dhcp4_config,
                    "dhcp_server_identifier"
                )
            );

        if (expiry)
        {
            info->lease_expires = g_ascii_strtoll(expiry, NULL, 10);

            if (lease_time)
            {
                info->lease_obtained =
                    info->lease_expires -
                    g_ascii_strtoll(lease_time, NULL, 10);
            }
        }
    }

    return info;
}

WinTCNetActivity wintc_net_device_get_activity(
    WinTCNetDevice* device
)
{
    return device->activity;
}

void wintc_net_device_get_traffic(
    WinTCNetDevice*  device,
    WinTCNetTraffic* traffic
)
{
    // The poll only runs while connected, read fresh numbers for anyone
    // else
    //
    if (!(device->id_activity))
    {
        device->traffic_valid =
            net_read_traffic(
                wintc_net_device_get_interface(device),
                &(device->traffic)
            );
    }

    if (device->traffic_valid)
    {
        *traffic = device->traffic;
    }
    else
    {
        memset(traffic, 0, sizeof(WinTCNetTraffic));
    }
}

gboolean wintc_net_device_get_radio_enabled(
    WinTCNetDevice* device
)
{
    if (device->kind != WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        return TRUE;
    }

    return nm_client_wireless_get_enabled(device->nm_client) &&
           nm_client_wireless_hardware_get_enabled(device->nm_client);
}

gboolean wintc_net_device_is_scanning(
    WinTCNetDevice* device
)
{
    return device->scanning;
}

WinTCWlanNetwork* wintc_net_device_get_active_network(
    WinTCNetDevice* device
)
{
    for (GList* iter = device->networks; iter; iter = iter->next)
    {
        WinTCWlanNetwork* network = (WinTCWlanNetwork*) iter->data;

        if (network->in_use)
        {
            return network;
        }
    }

    return NULL;
}

GList* wintc_net_device_get_networks(
    WinTCNetDevice* device
)
{
    return device->networks;
}

WinTCWlanNetwork* wintc_net_device_find_network(
    WinTCNetDevice* device,
    const gchar*    ssid
)
{
    for (GList* iter = device->networks; iter; iter = iter->next)
    {
        WinTCWlanNetwork* network = (WinTCWlanNetwork*) iter->data;

        if (g_strcmp0(network->ssid, ssid) == 0)
        {
            return network;
        }
    }

    return NULL;
}

void wintc_net_device_request_scan(
    WinTCNetDevice* device
)
{
    if (
        device->kind != WINTC_NET_DEVICE_KIND_WIRELESS ||
        device->scanning                               ||
        !wintc_net_device_get_radio_enabled(device)
    )
    {
        return;
    }

    device->scanning = TRUE;

    nm_device_wifi_request_scan_async(
        NM_DEVICE_WIFI(device->nm_device),
        NULL,
        on_nm_device_request_scan_ready,
        g_object_ref(device)
    );

    // NetworkManager tells us that the scan is done by updating last-scan,
    // don't sit in the scanning state forever if that never happens
    //
    device->id_scan_timeout =
        g_timeout_add_seconds(
            NET_SCAN_TIMEOUT_S,
            on_timeout_scan,
            device
        );

    g_signal_emit(
        device,
        wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
        0
    );
}

void wintc_net_device_connect_wlan_async(
    WinTCNetDevice*     device,
    const gchar*        ssid,
    const gchar*        key,
    GCancellable*       cancellable,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask*              task =
        g_task_new(device, cancellable, callback, user_data);
    WinTCNetActionData* data = g_new0(WinTCNetActionData, 1);

    data->cancellable = cancellable ? g_object_ref(cancellable) : NULL;
    data->timeout_s   = NET_CONNECT_TIMEOUT_S;

    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        data,
        (GDestroyNotify) net_action_data_free
    );

    WinTCWlanNetwork* network = wintc_net_device_find_network(device, ssid);

    if (!network)
    {
        net_action_complete(
            task,
            g_error_new(
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_NOT_FOUND,
                "Network '%s' is not in range.",
                ssid
            )
        );
        return;
    }

    data->ap_path = g_strdup(network->ap_path);

    // Preferred network - reuse the profile, updating the key if we were
    // given one
    //
    NMRemoteConnection* profile =
        network->profile_uuid ?
            nm_client_get_connection_by_uuid(
                device->nm_client,
                network->profile_uuid
            ) :
            NULL;

    if (profile)
    {
        data->connection = g_object_ref(profile);

        if (key)
        {
            NMConnection* updated =
                nm_simple_connection_new_clone(NM_CONNECTION(profile));

            net_set_wlan_key(updated, network->security, key);

            nm_remote_connection_update2(
                profile,
                nm_connection_to_dbus(updated, NM_CONNECTION_SERIALIZE_ALL),
                NM_SETTINGS_UPDATE2_FLAG_TO_DISK,
                NULL,
                cancellable,
                on_nm_remote_connection_update2_ready,
                task
            );

            g_object_unref(updated);
        }
        else
        {
            net_activate_connection(task);
        }

        return;
    }

    // New network, NetworkManager fills in whatever we don't specify
    //
    NMConnection* connection = net_build_wlan_connection(network, key);

    nm_client_add_and_activate_connection_async(
        device->nm_client,
        connection,
        device->nm_device,
        network->ap_path,
        cancellable,
        on_nm_client_add_and_activate_ready,
        task
    );

    g_object_unref(connection);
}

void wintc_net_device_disconnect_async(
    WinTCNetDevice*     device,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask* task = g_task_new(device, NULL, callback, user_data);

    nm_device_disconnect_async(
        device->nm_device,
        NULL,
        on_nm_device_disconnect_ready,
        task
    );
}

void wintc_net_device_repair_async(
    WinTCNetDevice*     device,
    GCancellable*       cancellable,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask*              task =
        g_task_new(device, cancellable, callback, user_data);
    WinTCNetActionData* data = g_new0(WinTCNetActionData, 1);

    data->cancellable = cancellable ? g_object_ref(cancellable) : NULL;
    data->timeout_s   = NET_REPAIR_TIMEOUT_S;

    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        data,
        (GDestroyNotify) net_action_data_free
    );

    // Repairing re-activates the connection, which gets a fresh DHCP lease
    // and re-associates on wireless
    //
    NMActiveConnection* active_connection =
        nm_device_get_active_connection(device->nm_device);
    NMRemoteConnection* connection =
        active_connection ?
            nm_active_connection_get_connection(active_connection) :
            NULL;

    if (!connection)
    {
        net_action_complete(
            task,
            g_error_new_literal(
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_NOT_FOUND,
                "The device has no active connection."
            )
        );
        return;
    }

    data->connection = g_object_ref(connection);

    if (NM_IS_DEVICE_WIFI(device->nm_device))
    {
        NMAccessPoint* ap =
            nm_device_wifi_get_active_access_point(
                NM_DEVICE_WIFI(device->nm_device)
            );

        if (ap)
        {
            data->ap_path = g_strdup(nm_object_get_path(NM_OBJECT(ap)));
        }
    }

    net_activate_connection(task);
}

void wintc_net_device_set_enabled_async(
    WinTCNetDevice*     device,
    gboolean            enabled,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask* task = g_task_new(device, NULL, callback, user_data);

    // Wireless adapters are switched off by the radio, the same as the
    // Wi-Fi switch in the Debian Xfce desktop's network applet
    //
    if (device->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        nm_client_dbus_set_property(
            device->nm_client,
            NM_DBUS_PATH,
            NM_DBUS_INTERFACE,
            "WirelessEnabled",
            g_variant_new_boolean(enabled),
            -1,
            NULL,
            on_nm_client_set_property_ready,
            task
        );

        return;
    }

    // Wired adapters - disconnecting stops NetworkManager from bringing the
    // connection back automatically, activating the device lets it pick the
    // best connection again
    //
    if (enabled)
    {
        nm_client_activate_connection_async(
            device->nm_client,
            NULL,
            device->nm_device,
            NULL,
            NULL,
            on_nm_client_activate_connection_ready,
            task
        );
    }
    else
    {
        nm_device_disconnect_async(
            device->nm_device,
            NULL,
            on_nm_device_disconnect_ready,
            task
        );
    }
}

gboolean wintc_net_device_action_finish(
    WINTC_UNUSED(WinTCNetDevice* device),
    GAsyncResult* res,
    GError**      error
)
{
    return g_task_propagate_boolean(G_TASK(res), error);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_net_device_queue_update(
    WinTCNetDevice* device,
    gboolean        networks
)
{
    device->update_networks |= networks;

    if (device->id_update)
    {
        return;
    }

    device->id_update =
        g_timeout_add(
            NET_UPDATE_DELAY_MS,
            on_timeout_update,
            device
        );
}

static void wintc_net_device_update_networks(
    WinTCNetDevice* device
)
{
    g_list_free_full(
        g_steal_pointer(&(device->networks)),
        (GDestroyNotify) wintc_wlan_network_free
    );

    if (
        device->kind != WINTC_NET_DEVICE_KIND_WIRELESS ||
        !wintc_net_device_get_radio_enabled(device)
    )
    {
        return;
    }

    NMDeviceWifi*    nm_wifi   = NM_DEVICE_WIFI(device->nm_device);
    const GPtrArray* aps       = nm_device_wifi_get_access_points(nm_wifi);
    NMAccessPoint*   active_ap =
        nm_device_wifi_get_active_access_point(nm_wifi);
    gboolean         activated =
        nm_device_get_state(device->nm_device) == NM_DEVICE_STATE_ACTIVATED;

    // XP lists networks, not access points - pick the best AP for each SSID
    //
    GHashTable* map_networks = g_hash_table_new(g_str_hash, g_str_equal);

    for (guint i = 0; aps && i < aps->len; i++)
    {
        NMAccessPoint* ap   = g_ptr_array_index(aps, i);
        GBytes*        ssid = nm_access_point_get_ssid(ap);

        if (!ssid || !g_bytes_get_size(ssid))
        {
            continue; // FIXME: Hidden networks are not listed
        }

        gchar* ssid_utf8 =
            nm_utils_ssid_to_utf8(
                g_bytes_get_data(ssid, NULL),
                g_bytes_get_size(ssid)
            );

        WinTCWlanNetwork* network =
            g_hash_table_lookup(map_networks, ssid_utf8);
        gint strength = nm_access_point_get_strength(ap);
        gboolean is_active = ap == active_ap;

        if (!network)
        {
            network = g_new0(WinTCWlanNetwork, 1);

            network->ssid   = ssid_utf8;
            network->signal = -1;

            g_hash_table_insert(map_networks, network->ssid, network);

            device->networks = g_list_prepend(device->networks, network);
        }
        else
        {
            g_free(ssid_utf8);
        }

        // The AP we are associated with wins, otherwise the strongest
        //
        if ((is_active || strength > network->signal) && !(network->in_use))
        {
            g_free(network->ap_path);
            g_clear_pointer(&(network->ssid_raw), g_bytes_unref);

            network->ap_path  = g_strdup(nm_object_get_path(NM_OBJECT(ap)));
            network->ssid_raw = g_bytes_ref(ssid);
            network->signal   = strength;
            network->rate     = nm_access_point_get_max_bitrate(ap) / 1000;
            network->security = net_get_ap_security(ap);
            network->in_use   = is_active && activated;
        }
    }

    g_hash_table_unref(map_networks);

    // Look up the preferred networks
    //
    for (GList* iter = device->networks; iter; iter = iter->next)
    {
        WinTCWlanNetwork*   network = (WinTCWlanNetwork*) iter->data;
        NMRemoteConnection* profile =
            net_find_wlan_profile(device, network->ssid_raw);

        if (profile)
        {
            NMSettingConnection* s_con =
                nm_connection_get_setting_connection(NM_CONNECTION(profile));

            network->profile_uuid        =
                g_strdup(nm_connection_get_uuid(NM_CONNECTION(profile)));
            network->profile_autoconnect =
                nm_setting_connection_get_autoconnect(s_con);
        }
    }

    device->networks =
        g_list_sort(device->networks, wintc_wlan_network_compare);
}

static void wintc_net_device_update_state(
    WinTCNetDevice* device
)
{
    NMDeviceState nm_state  = nm_device_get_state(device->nm_device);
    WinTCNetState new_state = WINTC_NET_STATE_DISCONNECTED;
    gboolean      radio     = wintc_net_device_get_radio_enabled(device);

    switch (nm_state)
    {
        case NM_DEVICE_STATE_UNKNOWN:
        case NM_DEVICE_STATE_UNMANAGED:
            new_state = WINTC_NET_STATE_UNAVAILABLE;
            break;

        case NM_DEVICE_STATE_UNAVAILABLE:
            // Wired - the cable is unplugged
            // Wireless - the radio is off, or the adapter is unusable
            //
            new_state =
                radio ?
                    WINTC_NET_STATE_UNAVAILABLE :
                    WINTC_NET_STATE_DISABLED;
            break;

        case NM_DEVICE_STATE_DISCONNECTED:
        case NM_DEVICE_STATE_FAILED:
            if (!radio)
            {
                new_state = WINTC_NET_STATE_DISABLED;
            }
            else if (
                device->kind == WINTC_NET_DEVICE_KIND_WIRED &&
                !nm_device_get_autoconnect(device->nm_device)
            )
            {
                // Disconnected by the user, which is what 'Disable' does
                // for wired adapters
                //
                new_state = WINTC_NET_STATE_DISABLED;
            }
            else
            {
                new_state = WINTC_NET_STATE_DISCONNECTED;
            }
            break;

        case NM_DEVICE_STATE_PREPARE:
        case NM_DEVICE_STATE_CONFIG:
        case NM_DEVICE_STATE_NEED_AUTH:
            new_state = WINTC_NET_STATE_CONNECTING;
            break;

        case NM_DEVICE_STATE_IP_CONFIG:
        case NM_DEVICE_STATE_IP_CHECK:
        case NM_DEVICE_STATE_SECONDARIES:
            new_state = WINTC_NET_STATE_ACQUIRING;
            break;

        case NM_DEVICE_STATE_ACTIVATED:
            // XP says 'Limited or no connectivity' when the network did not
            // give us an address and only a link-local one is left - it
            // never checks whether the internet is reachable, so NM's
            // connectivity state is deliberately not used here
            //
            if (
                net_ip_config_has_routable_address(
                    nm_device_get_ip4_config(device->nm_device)
                ) ||
                net_ip_config_has_routable_address(
                    nm_device_get_ip6_config(device->nm_device)
                )
            )
            {
                new_state = WINTC_NET_STATE_CONNECTED;
            }
            else
            {
                new_state = WINTC_NET_STATE_LIMITED;
            }

            break;

        case NM_DEVICE_STATE_DEACTIVATING:
            new_state = WINTC_NET_STATE_DISCONNECTING;
            break;
    }

    // Track how long we have been connected for the status window
    //
    gboolean was_connected = wintc_net_device_is_connected(device);

    device->state = new_state;

    if (wintc_net_device_is_connected(device))
    {
        if (!(device->state_known))
        {
            // Already connected when we started, the profile remembers when
            // it was last activated
            //
            NMActiveConnection* active_connection =
                nm_device_get_active_connection(device->nm_device);
            NMRemoteConnection* connection =
                active_connection ?
                    nm_active_connection_get_connection(active_connection) :
                    NULL;
            NMSettingConnection* s_con =
                connection ?
                    nm_connection_get_setting_connection(
                        NM_CONNECTION(connection)
                    ) :
                    NULL;
            guint64 timestamp =
                s_con ? nm_setting_connection_get_timestamp(s_con) : 0;
            gint64  now       = g_get_real_time() / G_USEC_PER_SEC;

            device->connected_since =
                timestamp && (gint64) timestamp <= now ?
                    (gint64) timestamp :
                    now;
        }
        else if (!was_connected)
        {
            device->connected_since = g_get_real_time() / G_USEC_PER_SEC;
        }
    }
    else
    {
        device->connected_since = 0;
    }

    device->state_known = TRUE;

    // Traffic is only interesting while there is a link
    //
    wintc_net_device_set_traffic_polling(
        device,
        new_state == WINTC_NET_STATE_CONNECTING ||
        new_state == WINTC_NET_STATE_ACQUIRING  ||
        wintc_net_device_is_connected(device)
    );
}

static void wintc_net_device_update_traffic(
    WinTCNetDevice* device
)
{
    WinTCNetTraffic  traffic;
    WinTCNetActivity activity = WINTC_NET_ACTIVITY_NONE;

    if (
        !net_read_traffic(
            wintc_net_device_get_interface(device),
            &traffic
        )
    )
    {
        device->traffic_valid = FALSE;
        return;
    }

    if (device->traffic_valid)
    {
        if (traffic.tx_bytes != device->traffic.tx_bytes)
        {
            activity |= WINTC_NET_ACTIVITY_TRANSMIT;
        }

        if (traffic.rx_bytes != device->traffic.rx_bytes)
        {
            activity |= WINTC_NET_ACTIVITY_RECEIVE;
        }
    }

    device->traffic       = traffic;
    device->traffic_valid = TRUE;

    if (activity != device->activity)
    {
        device->activity = activity;

        g_signal_emit(
            device,
            wintc_net_device_signals[SIGNAL_ACTIVITY_CHANGED],
            0
        );
    }
}

static void wintc_net_device_set_traffic_polling(
    WinTCNetDevice* device,
    gboolean        enabled
)
{
    if (enabled)
    {
        if (!(device->id_activity))
        {
            device->traffic_valid = FALSE;

            wintc_net_device_update_traffic(device);

            device->id_activity =
                g_timeout_add(
                    NET_ACTIVITY_INTERVAL_MS,
                    on_timeout_activity,
                    device
                );
        }

        return;
    }

    g_clear_handle_id(&(device->id_activity), g_source_remove);

    if (device->activity != WINTC_NET_ACTIVITY_NONE)
    {
        device->activity = WINTC_NET_ACTIVITY_NONE;

        g_signal_emit(
            device,
            wintc_net_device_signals[SIGNAL_ACTIVITY_CHANGED],
            0
        );
    }
}

static void net_action_complete(
    GTask*  task,
    GError* error
)
{
    WinTCNetActionData* data = g_task_get_task_data(task);

    if (data)
    {
        if (data->completed)
        {
            g_clear_error(&error);
            return;
        }

        data->completed = TRUE;

        if (data->active_connection)
        {
            g_clear_signal_handler(
                &(data->sigid_ac_state),
                data->active_connection
            );
        }

        if (data->cancellable)
        {
            g_clear_signal_handler(
                &(data->sigid_cancelled),
                data->cancellable
            );
        }

        g_clear_handle_id(&(data->id_timeout), g_source_remove);
        g_clear_handle_id(&(data->id_cancel),  g_source_remove);
    }

    if (error)
    {
        g_task_return_error(task, error);
    }
    else
    {
        g_task_return_boolean(task, TRUE);
    }

    g_object_unref(task);
}

static void net_action_data_free(
    WinTCNetActionData* data
)
{
    g_clear_object(&(data->active_connection));
    g_clear_object(&(data->cancellable));
    g_clear_object(&(data->connection));
    g_free(data->ap_path);
    g_free(data);
}

static void net_action_watch(
    GTask*              task,
    NMActiveConnection* active_connection
)
{
    WinTCNetActionData* data = g_task_get_task_data(task);

    data->active_connection = g_object_ref(active_connection);

    switch (nm_active_connection_get_state(active_connection))
    {
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
            net_action_complete(task, NULL);
            return;

        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
            net_action_complete(
                task,
                net_error_from_reason(
                    nm_active_connection_get_state_reason(active_connection)
                )
            );
            return;

        default:
            break;
    }

    data->sigid_ac_state =
        g_signal_connect(
            active_connection,
            "state-changed",
            G_CALLBACK(on_active_connection_state_changed),
            task
        );

    data->id_timeout =
        g_timeout_add_seconds(
            data->timeout_s,
            on_action_timeout,
            task
        );

    if (data->cancellable)
    {
        data->sigid_cancelled =
            g_signal_connect(
                data->cancellable,
                "cancelled",
                G_CALLBACK(on_action_cancelled),
                task
            );

        if (g_cancellable_is_cancelled(data->cancellable))
        {
            on_action_cancelled(data->cancellable, task);
        }
    }
}

static void net_activate_connection(
    GTask* task
)
{
    WinTCNetDevice*     device =
        WINTC_NET_DEVICE(g_task_get_source_object(task));
    WinTCNetActionData* data   = g_task_get_task_data(task);

    nm_client_activate_connection_async(
        device->nm_client,
        NM_CONNECTION(data->connection),
        device->nm_device,
        data->ap_path,
        data->cancellable,
        on_nm_client_activate_connection_ready,
        task
    );
}

static NMConnection* net_build_wlan_connection(
    WinTCWlanNetwork* network,
    const gchar*      key
)
{
    NMConnection*        connection = nm_simple_connection_new();
    NMSettingConnection* s_con      =
        NM_SETTING_CONNECTION(nm_setting_connection_new());
    NMSettingWireless*   s_wireless =
        NM_SETTING_WIRELESS(nm_setting_wireless_new());
    gchar*               uuid       = nm_utils_uuid_generate();

    // Same as what the network applet creates - the profile is named after
    // the network and connects automatically from now on
    //
    g_object_set(
        s_con,
        NM_SETTING_CONNECTION_ID,          network->ssid,
        NM_SETTING_CONNECTION_UUID,        uuid,
        NM_SETTING_CONNECTION_TYPE,        NM_SETTING_WIRELESS_SETTING_NAME,
        NM_SETTING_CONNECTION_AUTOCONNECT, TRUE,
        NULL
    );
    g_object_set(
        s_wireless,
        NM_SETTING_WIRELESS_SSID, network->ssid_raw,
        NM_SETTING_WIRELESS_MODE, NM_SETTING_WIRELESS_MODE_INFRA,
        NULL
    );

    nm_connection_add_setting(connection, NM_SETTING(s_con));
    nm_connection_add_setting(connection, NM_SETTING(s_wireless));

    g_free(uuid);

    if (network->security != WINTC_WLAN_SECURITY_NONE)
    {
        NMSetting*   s_security = nm_setting_wireless_security_new();
        const gchar* key_mgmt;

        switch (network->security)
        {
            case WINTC_WLAN_SECURITY_OWE:  key_mgmt = "owe";     break;
            case WINTC_WLAN_SECURITY_WEP:  key_mgmt = "none";    break;
            case WINTC_WLAN_SECURITY_WPA3: key_mgmt = "sae";     break;
            default:                       key_mgmt = "wpa-psk"; break;
        }

        g_object_set(
            s_security,
            NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, key_mgmt,
            NULL
        );

        nm_connection_add_setting(connection, s_security);

        if (key)
        {
            net_set_wlan_key(connection, network->security, key);
        }
    }

    return connection;
}

static NMRemoteConnection* net_find_wlan_profile(
    WinTCNetDevice* device,
    GBytes*         ssid
)
{
    const GPtrArray*    connections =
        nm_client_get_connections(device->nm_client);
    NMRemoteConnection* best        = NULL;
    guint64             best_ts     = 0;

    if (!ssid)
    {
        return NULL;
    }

    for (guint i = 0; connections && i < connections->len; i++)
    {
        NMConnection*      connection = g_ptr_array_index(connections, i);
        NMSettingWireless* s_wireless =
            nm_connection_get_setting_wireless(connection);

        if (!s_wireless)
        {
            continue;
        }

        // Skip hotspots and the like
        //
        const gchar* mode = nm_setting_wireless_get_mode(s_wireless);

        if (mode && g_strcmp0(mode, NM_SETTING_WIRELESS_MODE_INFRA) != 0)
        {
            continue;
        }

        GBytes* profile_ssid = nm_setting_wireless_get_ssid(s_wireless);

        if (!profile_ssid || !g_bytes_equal(profile_ssid, ssid))
        {
            continue;
        }

        if (
            !nm_device_connection_compatible(
                device->nm_device,
                connection,
                NULL
            )
        )
        {
            continue;
        }

        // Most recently used wins if there are several
        //
        guint64 ts =
            nm_setting_connection_get_timestamp(
                nm_connection_get_setting_connection(connection)
            );

        if (!best || ts > best_ts)
        {
            best    = NM_REMOTE_CONNECTION(connection);
            best_ts = ts;
        }
    }

    return best;
}

static GError* net_error_from_reason(
    NMActiveConnectionStateReason reason
)
{
    switch (reason)
    {
        case NM_ACTIVE_CONNECTION_STATE_REASON_NO_SECRETS:
        case NM_ACTIVE_CONNECTION_STATE_REASON_LOGIN_FAILED:
            return g_error_new_literal(
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_SECRETS_REQUIRED,
                "The network key is missing or incorrect."
            );

        default:
            return g_error_new(
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_FAILED,
                "Activation failed (reason %d).",
                (gint) reason
            );
    }
}

static WinTCWlanSecurity net_get_ap_security(
    NMAccessPoint* ap
)
{
    NM80211ApFlags         flags = nm_access_point_get_flags(ap);
    NM80211ApSecurityFlags wpa   = nm_access_point_get_wpa_flags(ap);
    NM80211ApSecurityFlags rsn   = nm_access_point_get_rsn_flags(ap);

    if (
        (wpa | rsn) & (
            NM_802_11_AP_SEC_KEY_MGMT_802_1X |
            NM_802_11_AP_SEC_KEY_MGMT_EAP_SUITE_B_192
        )
    )
    {
        return WINTC_WLAN_SECURITY_ENTERPRISE;
    }

    if (rsn & NM_802_11_AP_SEC_KEY_MGMT_PSK)
    {
        // WPA2/WPA3 transition networks accept the WPA2 key just fine
        //
        return WINTC_WLAN_SECURITY_WPA2;
    }

    if (rsn & NM_802_11_AP_SEC_KEY_MGMT_SAE)
    {
        return WINTC_WLAN_SECURITY_WPA3;
    }

    if (wpa & NM_802_11_AP_SEC_KEY_MGMT_PSK)
    {
        return WINTC_WLAN_SECURITY_WPA;
    }

    if (
        rsn & (
            NM_802_11_AP_SEC_KEY_MGMT_OWE |
            NM_802_11_AP_SEC_KEY_MGMT_OWE_TM
        )
    )
    {
        return WINTC_WLAN_SECURITY_OWE;
    }

    if (flags & NM_802_11_AP_FLAGS_PRIVACY)
    {
        return WINTC_WLAN_SECURITY_WEP;
    }

    return WINTC_WLAN_SECURITY_NONE;
}

static gboolean net_ip_config_has_routable_address(
    NMIPConfig* ip_config
)
{
    if (!ip_config)
    {
        return FALSE;
    }

    GPtrArray* addresses = nm_ip_config_get_addresses(ip_config);

    for (guint i = 0; addresses && i < addresses->len; i++)
    {
        NMIPAddress* address = g_ptr_array_index(addresses, i);
        const gchar* text    = nm_ip_address_get_address(address);

        if (nm_ip_address_get_family(address) == AF_INET)
        {
            if (!g_str_has_prefix(text, "169.254."))
            {
                return TRUE;
            }
        }
        else
        {
            GInetAddress* inet_address = g_inet_address_new_from_string(text);
            gboolean      link_local   =
                !inet_address ||
                g_inet_address_get_is_link_local(inet_address);

            g_clear_object(&inet_address);

            if (!link_local)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static gboolean net_read_traffic(
    const gchar*     iface,
    WinTCNetTraffic* traffic
)
{
    if (!iface)
    {
        return FALSE;
    }

    // The counters in sysfs are 64-bit, unlike the ones from getifaddrs()
    //
    static const gchar* s_counters[] = {
        "tx_bytes",
        "rx_bytes",
        "tx_packets",
        "rx_packets"
    };

    guint64* values[] = {
        &(traffic->tx_bytes),
        &(traffic->rx_bytes),
        &(traffic->tx_packets),
        &(traffic->rx_packets)
    };

    for (gsize i = 0; i < G_N_ELEMENTS(s_counters); i++)
    {
        gchar* contents = NULL;
        gchar* path     =
            g_build_filename(
                "/sys/class/net",
                iface,
                "statistics",
                s_counters[i],
                NULL
            );

        gboolean success = g_file_get_contents(path, &contents, NULL, NULL);

        if (success)
        {
            *(values[i]) = g_ascii_strtoull(contents, NULL, 10);
        }

        g_free(contents);
        g_free(path);

        if (!success)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static void net_set_wlan_key(
    NMConnection*     connection,
    WinTCWlanSecurity security,
    const gchar*      key
)
{
    NMSettingWirelessSecurity* s_security =
        nm_connection_get_setting_wireless_security(connection);

    if (!s_security)
    {
        s_security =
            NM_SETTING_WIRELESS_SECURITY(nm_setting_wireless_security_new());

        nm_connection_add_setting(connection, NM_SETTING(s_security));
    }

    // Keys are stored with the profile, as nmcli and nm-applet do it - there
    // is no keyring agent behind us
    //
    if (security == WINTC_WLAN_SECURITY_WEP)
    {
        gsize           len      = strlen(key);
        NMWepKeyType    key_type = NM_WEP_KEY_TYPE_PASSPHRASE;

        if (
            nm_utils_wep_key_valid(key, NM_WEP_KEY_TYPE_KEY) &&
            (len == 5 || len == 10 || len == 13 || len == 26)
        )
        {
            key_type = NM_WEP_KEY_TYPE_KEY;
        }

        g_object_set(
            s_security,
            NM_SETTING_WIRELESS_SECURITY_KEY_MGMT,      "none",
            NM_SETTING_WIRELESS_SECURITY_WEP_KEY_TYPE,  key_type,
            NM_SETTING_WIRELESS_SECURITY_WEP_KEY_FLAGS,
                NM_SETTING_SECRET_FLAG_NONE,
            NULL
        );

        nm_setting_wireless_security_set_wep_key(s_security, 0, key);

        return;
    }

    g_object_set(
        s_security,
        NM_SETTING_WIRELESS_SECURITY_PSK,       key,
        NM_SETTING_WIRELESS_SECURITY_PSK_FLAGS, NM_SETTING_SECRET_FLAG_NONE,
        NULL
    );
}

//
// CALLBACKS
//
static void on_action_cancelled(
    WINTC_UNUSED(GCancellable* cancellable),
    gpointer user_data
)
{
    WinTCNetActionData* data = g_task_get_task_data(G_TASK(user_data));

    // Can't complete the task from inside the cancelled signal, the handler
    // would be disconnected during its own emission
    //
    if (!(data->id_cancel))
    {
        data->id_cancel = g_idle_add(on_action_cancelled_idle, user_data);
    }
}

static gboolean on_action_cancelled_idle(
    gpointer user_data
)
{
    GTask*              task   = G_TASK(user_data);
    WinTCNetActionData* data   = g_task_get_task_data(task);
    WinTCNetDevice*     device =
        WINTC_NET_DEVICE(g_task_get_source_object(task));

    data->id_cancel = 0;

    // Stop NetworkManager from carrying on in the background
    //
    if (data->active_connection)
    {
        nm_client_deactivate_connection_async(
            device->nm_client,
            data->active_connection,
            NULL,
            NULL,
            NULL
        );
    }

    net_action_complete(
        task,
        g_error_new_literal(
            G_IO_ERROR,
            G_IO_ERROR_CANCELLED,
            "Cancelled."
        )
    );

    return G_SOURCE_REMOVE;
}

static gboolean on_action_timeout(
    gpointer user_data
)
{
    GTask*              task   = G_TASK(user_data);
    WinTCNetActionData* data   = g_task_get_task_data(task);
    WinTCNetDevice*     device =
        WINTC_NET_DEVICE(g_task_get_source_object(task));

    data->id_timeout = 0;

    if (data->active_connection)
    {
        nm_client_deactivate_connection_async(
            device->nm_client,
            data->active_connection,
            NULL,
            NULL,
            NULL
        );
    }

    net_action_complete(
        task,
        g_error_new_literal(
            G_IO_ERROR,
            G_IO_ERROR_TIMED_OUT,
            "Timed out waiting for the connection."
        )
    );

    return G_SOURCE_REMOVE;
}

static void on_active_connection_state_changed(
    WINTC_UNUSED(NMActiveConnection* active_connection),
    guint    state,
    guint    reason,
    gpointer user_data
)
{
    GTask* task = G_TASK(user_data);

    switch (state)
    {
        case NM_ACTIVE_CONNECTION_STATE_ACTIVATED:
            net_action_complete(task, NULL);
            break;

        case NM_ACTIVE_CONNECTION_STATE_DEACTIVATED:
            net_action_complete(
                task,
                net_error_from_reason(
                    (NMActiveConnectionStateReason) reason
                )
            );
            break;

        default:
            break;
    }
}

static void on_nm_client_activate_connection_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*              task  = G_TASK(user_data);
    GError*             error = NULL;
    NMActiveConnection* active_connection;

    active_connection =
        nm_client_activate_connection_finish(
            NM_CLIENT(source_object),
            res,
            &error
        );

    if (!active_connection)
    {
        net_action_complete(task, error);
        return;
    }

    // Enabling a wired adapter does not wait for the connection
    //
    if (!g_task_get_task_data(task))
    {
        g_object_unref(active_connection);
        net_action_complete(task, NULL);
        return;
    }

    net_action_watch(task, active_connection);

    g_object_unref(active_connection);
}

static void on_nm_client_add_and_activate_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*              task  = G_TASK(user_data);
    GError*             error = NULL;
    NMActiveConnection* active_connection;

    active_connection =
        nm_client_add_and_activate_connection_finish(
            NM_CLIENT(source_object),
            res,
            &error
        );

    if (!active_connection)
    {
        net_action_complete(task, error);
        return;
    }

    net_action_watch(task, active_connection);

    g_object_unref(active_connection);
}

static void on_nm_client_connections_changed(
    WINTC_UNUSED(NMClient* nm_client),
    WINTC_UNUSED(GObject*  connection),
    gpointer user_data
)
{
    wintc_net_device_queue_update(WINTC_NET_DEVICE(user_data), TRUE);
}

static void on_nm_client_notify(
    WINTC_UNUSED(GObject* self),
    GParamSpec* pspec,
    gpointer    user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    if (
        device->kind == WINTC_NET_DEVICE_KIND_WIRELESS &&
        (
            g_strcmp0(pspec->name, NM_CLIENT_WIRELESS_ENABLED) == 0 ||
            g_strcmp0(pspec->name, NM_CLIENT_WIRELESS_HARDWARE_ENABLED) == 0
        )
    )
    {
        wintc_net_device_queue_update(device, TRUE);
    }
}

static void on_nm_client_set_property_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*  task  = G_TASK(user_data);
    GError* error = NULL;

    if (
        nm_client_dbus_set_property_finish(
            NM_CLIENT(source_object),
            res,
            &error
        )
    )
    {
        g_task_return_boolean(task, TRUE);
    }
    else
    {
        g_task_return_error(task, error);
    }

    g_object_unref(task);
}

static void on_nm_device_access_points_changed(
    WINTC_UNUSED(NMDeviceWifi* nm_device),
    WINTC_UNUSED(GObject*      ap),
    gpointer user_data
)
{
    wintc_net_device_queue_update(WINTC_NET_DEVICE(user_data), TRUE);
}

static void on_nm_device_disconnect_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*  task  = G_TASK(user_data);
    GError* error = NULL;

    if (nm_device_disconnect_finish(NM_DEVICE(source_object), res, &error))
    {
        g_task_return_boolean(task, TRUE);
    }
    else
    {
        g_task_return_error(task, error);
    }

    g_object_unref(task);
}

static void on_nm_device_notify(
    WINTC_UNUSED(GObject* self),
    GParamSpec* pspec,
    gpointer    user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    gboolean networks =
        g_strcmp0(pspec->name, NM_DEVICE_STATE)                     == 0 ||
        g_strcmp0(pspec->name, NM_DEVICE_WIFI_ACTIVE_ACCESS_POINT) == 0;

    if (g_strcmp0(pspec->name, NM_DEVICE_WIFI_LAST_SCAN) == 0)
    {
        g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);

        device->scanning = FALSE;
        networks         = TRUE;
    }

    wintc_net_device_queue_update(device, networks);
}

static void on_nm_device_request_scan_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);
    GError*         error  = NULL;

    if (
        !nm_device_wifi_request_scan_finish(
            NM_DEVICE_WIFI(source_object),
            res,
            &error
        )
    )
    {
        // Usually just NetworkManager refusing to scan again so soon, the
        // list we have is recent anyway
        //
        WINTC_LOG_DEBUG("net: scan request failed: %s", error->message);
        g_clear_error(&error);

        if (device->scanning && device->nm_device)
        {
            g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);

            device->scanning = FALSE;

            g_signal_emit(
                device,
                wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
                0
            );
        }
    }

    g_object_unref(device);
}

static void on_nm_remote_connection_update2_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*    task   = G_TASK(user_data);
    GError*   error  = NULL;
    GVariant* result;

    result =
        nm_remote_connection_update2_finish(
            NM_REMOTE_CONNECTION(source_object),
            res,
            &error
        );

    if (!result)
    {
        net_action_complete(task, error);
        return;
    }

    g_variant_unref(result);

    net_activate_connection(task);
}

static gboolean on_timeout_activity(
    gpointer user_data
)
{
    wintc_net_device_update_traffic(WINTC_NET_DEVICE(user_data));

    return G_SOURCE_CONTINUE;
}

static gboolean on_timeout_scan(
    gpointer user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    device->id_scan_timeout = 0;
    device->scanning        = FALSE;

    g_signal_emit(
        device,
        wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
        0
    );

    return G_SOURCE_REMOVE;
}

static gboolean on_timeout_update(
    gpointer user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    gboolean networks = device->update_networks;

    device->id_update       = 0;
    device->update_networks = FALSE;

    wintc_net_device_update_state(device);

    if (networks)
    {
        wintc_net_device_update_networks(device);

        g_signal_emit(
            device,
            wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
            0
        );
    }

    g_signal_emit(
        device,
        wintc_net_device_signals[SIGNAL_STATE_CHANGED],
        0
    );

    return G_SOURCE_REMOVE;
}
