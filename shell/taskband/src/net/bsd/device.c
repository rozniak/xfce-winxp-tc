#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <ifaddrs.h>
#include <signal.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_media.h>
#include <net/route.h>
#include <net80211/ieee80211_ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <wintc/comgtk.h>

#include "../device.h"
#include "../util.h"
#include "backend.h"
#include "wpactrl.h"

//
// PRIVATE CONSTANTS
//
#define NET_ACTIVITY_INTERVAL_MS 500
#define NET_UPDATE_DELAY_MS      100

#define NET_CONNECT_TIMEOUT_S 60
#define NET_REPAIR_TIMEOUT_S  45
#define NET_SCAN_TIMEOUT_S    15

#define NET_WPA_RETRY_S  5
#define NET_WLAN_POLL_S 10

// XP waits about a minute for DHCP before it settles for 'Limited or no
// connectivity', dhclient is started by devd(8) a moment after the link
// comes up
//
#define NET_DHCP_START_S   5
#define NET_DHCP_TIMEOUT_S 60

#define NET_WPA_CTRL_DIR   "/var/run/wpa_supplicant"
#define NET_PKEXEC_PATH    "/usr/local/bin/pkexec"
#define NET_DHCLIENT_PID   "/var/run/dhclient/dhclient.%s.pid"
#define NET_DHCLIENT_LEASE "/var/db/dhclient.leases.%s"
#define NET_RESOLV_CONF    "/etc/resolv.conf"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_INTERFACE,
    PROP_KIND,
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

// What the kernel and wpa_supplicant had to say about the adapter, gathered
// on a worker thread
//
typedef struct _WinTCNetSnapshot
{
    gboolean exists;
    gboolean up;
    gboolean link;
    gboolean routable;
    guint32  speed;
    gint     signal;
    gint64   dhclient_started;

    gboolean wpa;
    gchar*   wpa_state;
    gint     wpa_id;
    GList*   networks;
} WinTCNetSnapshot;

typedef struct _WinTCNetRefreshData
{
    gchar*   iface;
    gboolean wireless;
    gchar*   wpa_path;
} WinTCNetRefreshData;

// A connect or repair that waits for the adapter to come back up
//
typedef struct _WinTCNetActionData
{
    gint     network_id;
    gboolean created;
    gboolean associated;

    GCancellable* cancellable;
    gulong        sigid_cancelled;
    guint         id_timeout;
    guint         id_cancel;
    gboolean      completed;
} WinTCNetActionData;

typedef struct _WinTCNetConnectData
{
    gchar*            wpa_path;
    GBytes*           ssid;
    WinTCWlanSecurity security;
    gchar*            key;
    gint              profile_id;

    gint     network_id;
    gboolean created;
    gboolean already_connected;
} WinTCNetConnectData;

//
// FORWARD DECLARATIONS
//
static void wintc_net_device_constructed(
    GObject* object
);
static void wintc_net_device_dispose(
    GObject* object
);
static void wintc_net_device_finalize(
    GObject* object
);
static void wintc_net_device_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_net_device_apply_snapshot(
    WinTCNetDevice*   device,
    WinTCNetSnapshot* snapshot
);
static void wintc_net_device_check_action(
    WinTCNetDevice* device
);
static void wintc_net_device_queue_update(
    WinTCNetDevice* device,
    gboolean        networks
);
static void wintc_net_device_set_traffic_polling(
    WinTCNetDevice* device,
    gboolean        enabled
);
static void wintc_net_device_update_state(
    WinTCNetDevice* device
);
static void wintc_net_device_update_traffic(
    WinTCNetDevice* device
);
static void wintc_net_device_wpa_attach(
    WinTCNetDevice* device
);
static void wintc_net_device_wpa_detach(
    WinTCNetDevice* device
);

static void net_action_complete(
    WinTCNetDevice* device,
    GError*         error
);
static void net_action_data_free(
    WinTCNetActionData* data
);
static void net_admin_error(
    GSubprocess* proc,
    GError**     error
);
static void net_action_start(
    WinTCNetDevice* device,
    GTask*          task
);
static void net_connect_data_free(
    WinTCNetConnectData* data
);
static void net_connect_thread(
    GTask*        task,
    gpointer      source_object,
    gpointer      task_data,
    GCancellable* cancellable
);
static gboolean net_dhclient_running(
    const gchar* iface,
    gint64*      started
);
static gchar* net_format_key(
    WinTCWlanSecurity security,
    const gchar*      key
);
static gchar* net_get_gateway(
    guint ifindex
);
static void net_run_netctl(
    WinTCNetDevice*     device,
    const gchar*        verb,
    GAsyncReadyCallback callback,
    GTask*              task
);
static void net_read_lease(
    const gchar*    iface,
    WinTCNetIpInfo* info
);
static gboolean net_read_traffic(
    const gchar*     iface,
    WinTCNetTraffic* traffic
);
static void net_read_wireless(
    gint              sock,
    const gchar*      iface,
    WinTCNetSnapshot* snapshot
);
static void net_read_wpa(
    const gchar*      wpa_path,
    WinTCNetSnapshot* snapshot
);
static void net_refresh_data_free(
    WinTCNetRefreshData* data
);
static void net_refresh_thread(
    GTask*        task,
    gpointer      source_object,
    gpointer      task_data,
    GCancellable* cancellable
);
static void net_run_wpa_commands(
    WinTCNetDevice*     device,
    const gchar* const* commands
);
static void net_run_wpa_thread(
    GTask*        task,
    gpointer      source_object,
    gpointer      task_data,
    GCancellable* cancellable
);
static gint net_signal_from_dbm(
    gint dbm
);
static void net_snapshot_free(
    WinTCNetSnapshot* snapshot
);
static WinTCNetSnapshot* net_snapshot_take(
    WinTCNetRefreshData* data
);
static GBytes* net_wpa_decode_text(
    const gchar* text
);
static WinTCWlanSecurity net_wpa_flags_to_security(
    const gchar* flags
);
static gchar* net_wpa_get_field(
    const gchar* reply,
    const gchar* key
);

static gboolean on_action_cancelled_idle(
    gpointer user_data
);
static void on_action_cancelled(
    GCancellable* cancellable,
    gpointer      user_data
);
static gboolean on_action_timeout(
    gpointer user_data
);
static void on_connect_thread_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_dhclient_restart_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_ifconfig_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_refresh_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_simple_wpa_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_scan_request_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static gboolean on_timeout_activity(
    gpointer user_data
);
static gboolean on_timeout_recheck(
    gpointer user_data
);
static gboolean on_timeout_scan(
    gpointer user_data
);
static gboolean on_timeout_update(
    gpointer user_data
);
static gboolean on_timeout_wlan_poll(
    gpointer user_data
);
static gboolean on_timeout_wpa_retry(
    gpointer user_data
);
static void on_wpa_commands_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_wpa_monitor_closed(
    WinTCWpaMonitor* monitor,
    gpointer         user_data
);
static void on_wpa_monitor_event(
    WinTCWpaMonitor* monitor,
    const gchar*     text,
    gpointer         user_data
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

    gchar*             iface;
    WinTCNetDeviceKind kind;

    // State
    //
    WinTCNetState     state;
    gint64            connected_since;
    gint64            link_since;
    gboolean          state_known;
    WinTCNetSnapshot* snapshot;

    GCancellable* cancellable;
    guint         id_update;
    guint         id_recheck;
    gboolean      update_networks;
    gboolean      refreshing;
    gboolean      refresh_queued;

    GTask* action;

    // Wireless
    //
    gchar*           wpa_path;
    WinTCWpaMonitor* wpa_monitor;
    gboolean         wpa_warned;
    guint            id_wpa_retry;
    guint            id_wlan_poll;

    gulong sigid_wpa_closed;
    gulong sigid_wpa_event;

    GList*   networks;
    gchar*   networks_key;
    gboolean scanning;
    guint    id_scan_timeout;

    // Traffic
    //
    WinTCNetTraffic  traffic;
    gboolean         traffic_valid;
    WinTCNetActivity activity;
    guint            id_activity;
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
    object_class->finalize     = wintc_net_device_finalize;
    object_class->set_property = wintc_net_device_set_property;

    wintc_net_device_properties[PROP_INTERFACE] =
        g_param_spec_string(
            "interface",
            "Interface",
            "The name of the network interface.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );
    wintc_net_device_properties[PROP_KIND] =
        g_param_spec_int(
            "kind",
            "Kind",
            "Whether the adapter is wired or wireless.",
            WINTC_NET_DEVICE_KIND_WIRED,
            WINTC_NET_DEVICE_KIND_WIRELESS,
            WINTC_NET_DEVICE_KIND_WIRED,
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
    WinTCNetDevice* self
)
{
    self->cancellable = g_cancellable_new();
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_net_device_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_net_device_parent_class))->constructed(object);

    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    // wpa_supplicant does the scanning and associating, it's started by
    // rc.d for wlan interfaces with WPA in their ifconfig_ line
    //
    if (device->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        device->wpa_path =
            g_build_filename(NET_WPA_CTRL_DIR, device->iface, NULL);

        wintc_net_device_wpa_attach(device);
    }

    // Take the first look straight away so that the tray icon is right
    // from the start, the kernel answers quickly
    //
    WinTCNetRefreshData data = {
        .iface    = device->iface,
        .wireless = device->kind == WINTC_NET_DEVICE_KIND_WIRELESS,
        .wpa_path = device->wpa_monitor ? device->wpa_path : NULL
    };

    device->update_networks = TRUE;

    wintc_net_device_apply_snapshot(device, net_snapshot_take(&data));
}

static void wintc_net_device_dispose(
    GObject* object
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    g_cancellable_cancel(device->cancellable);

    if (device->action)
    {
        net_action_complete(
            device,
            g_error_new_literal(
                G_IO_ERROR,
                G_IO_ERROR_CANCELLED,
                "The adapter went away."
            )
        );
    }

    g_clear_handle_id(&(device->id_update),       g_source_remove);
    g_clear_handle_id(&(device->id_recheck),      g_source_remove);
    g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);
    g_clear_handle_id(&(device->id_activity),     g_source_remove);
    g_clear_handle_id(&(device->id_wpa_retry),    g_source_remove);
    g_clear_handle_id(&(device->id_wlan_poll),    g_source_remove);

    wintc_net_device_wpa_detach(device);

    g_list_free_full(
        g_steal_pointer(&(device->networks)),
        (GDestroyNotify) wintc_wlan_network_free
    );

    g_clear_pointer(&(device->snapshot), net_snapshot_free);
    g_clear_object(&(device->cancellable));

    (G_OBJECT_CLASS(wintc_net_device_parent_class))->dispose(object);
}

static void wintc_net_device_finalize(
    GObject* object
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(object);

    g_free(device->iface);
    g_free(device->wpa_path);
    g_free(device->networks_key);

    (G_OBJECT_CLASS(wintc_net_device_parent_class))->finalize(object);
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
        case PROP_INTERFACE:
            device->iface = g_value_dup_string(value);
            break;

        case PROP_KIND:
            device->kind = (WinTCNetDeviceKind) g_value_get_int(value);
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
    const gchar*       iface,
    WinTCNetDeviceKind kind
)
{
    return WINTC_NET_DEVICE(
        g_object_new(
            WINTC_TYPE_NET_DEVICE,
            "interface", iface,
            "kind",      (gint) kind,
            NULL
        )
    );
}

void wintc_net_device_refresh(
    WinTCNetDevice* device
)
{
    wintc_net_device_queue_update(device, FALSE);
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
    return device->iface;
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
    return device->snapshot ? device->snapshot->speed : 0;
}

const gchar* wintc_net_device_get_connection_uuid(
    WINTC_UNUSED(WinTCNetDevice* device)
)
{
    // Connection profiles are a NetworkManager thing, wpa_supplicant's
    // networks show up through the wireless network list instead
    //
    return NULL;
}

WinTCNetIpInfo* wintc_net_device_get_ip_info(
    WinTCNetDevice* device
)
{
    WinTCNetIpInfo* info = g_new0(WinTCNetIpInfo, 1);
    struct ifaddrs* ifap = NULL;
    guint           index = 0;
    gboolean        addresses =
        wintc_net_device_is_connected(device) ||
        device->state == WINTC_NET_STATE_ACQUIRING;

    if (getifaddrs(&ifap) != 0)
    {
        return info;
    }

    for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (!(ifa->ifa_addr) || g_strcmp0(ifa->ifa_name, device->iface))
        {
            continue;
        }

        if (ifa->ifa_addr->sa_family == AF_LINK)
        {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*) ifa->ifa_addr;
            guchar*             mac = (guchar*) LLADDR(sdl);

            index = sdl->sdl_index;

            if (sdl->sdl_alen == 6)
            {
                info->hwaddr =
                    g_strdup_printf(
                        "%02X:%02X:%02X:%02X:%02X:%02X",
                        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
                    );
            }
        }
        else if (
            addresses                           &&
            ifa->ifa_addr->sa_family == AF_INET &&
            !(info->address)
        )
        {
            gchar     buf[INET_ADDRSTRLEN];
            in_addr_t mask =
                ifa->ifa_netmask ?
                    ((struct sockaddr_in*) ifa->ifa_netmask)->sin_addr.s_addr :
                    0;

            inet_ntop(
                AF_INET,
                &(((struct sockaddr_in*) ifa->ifa_addr)->sin_addr),
                buf,
                sizeof(buf)
            );

            info->address = g_strdup(buf);
            info->prefix  = __builtin_popcount(ntohl(mask));
        }
    }

    freeifaddrs(ifap);

    if (!addresses)
    {
        return info;
    }

    info->gateway = net_get_gateway(index);

    // DNS servers are system wide on FreeBSD
    //
    gchar* contents = NULL;

    if (g_file_get_contents(NET_RESOLV_CONF, &contents, NULL, NULL))
    {
        GStrvBuilder* builder = g_strv_builder_new();
        gchar**       lines   = g_strsplit(contents, "\n", -1);

        for (gchar** line = lines; *line; line++)
        {
            gchar** words = g_strsplit_set(g_strstrip(*line), " \t", -1);

            if (
                g_strcmp0(words[0], "nameserver") == 0 &&
                words[1] && *(words[1])
            )
            {
                g_strv_builder_add(builder, words[1]);
            }

            g_strfreev(words);
        }

        info->dns = g_strv_builder_end(builder);

        g_strv_builder_unref(builder);
        g_strfreev(lines);
        g_free(contents);
    }

    info->dhcp = net_dhclient_running(device->iface, NULL);

    if (info->dhcp)
    {
        net_read_lease(device->iface, info);
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
            net_read_traffic(device->iface, &(device->traffic));
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
    // There's no separate radio switch, a wlan interface that is down is
    // switched off
    //
    if (device->kind != WINTC_NET_DEVICE_KIND_WIRELESS || !(device->snapshot))
    {
        return TRUE;
    }

    return device->snapshot->up;
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
        !(device->wpa_monitor)                         ||
        !wintc_net_device_get_radio_enabled(device)
    )
    {
        return;
    }

    device->scanning = TRUE;

    GTask* task =
        g_task_new(device, device->cancellable, on_scan_request_done, NULL);

    g_task_set_task_data(
        task,
        g_strdupv((gchar**) (const gchar*[]) { "SCAN", NULL }),
        (GDestroyNotify) g_strfreev
    );
    g_task_run_in_thread(task, net_run_wpa_thread);
    g_object_unref(task);

    // wpa_supplicant says when the scan is done, don't sit in the scanning
    // state forever if that never happens
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

    data->network_id  = -1;
    data->cancellable = cancellable ? g_object_ref(cancellable) : NULL;

    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        data,
        (GDestroyNotify) net_action_data_free
    );

    WinTCWlanNetwork* network = wintc_net_device_find_network(device, ssid);

    if (!network)
    {
        g_task_return_new_error(
            task,
            WINTC_NET_ERROR,
            WINTC_NET_ERROR_NOT_FOUND,
            "Network '%s' is not in range.",
            ssid
        );
        g_object_unref(task);
        return;
    }

    if (!(device->wpa_monitor))
    {
        g_task_return_new_error(
            task,
            WINTC_NET_ERROR,
            WINTC_NET_ERROR_FAILED,
            "wpa_supplicant is not running on %s.",
            device->iface
        );
        g_object_unref(task);
        return;
    }

    // Set the network up in wpa_supplicant on a worker thread, then wait
    // here for it to associate
    //
    WinTCNetConnectData* connect = g_new0(WinTCNetConnectData, 1);

    connect->wpa_path   = g_strdup(device->wpa_path);
    connect->ssid       = g_bytes_ref(network->ssid_raw);
    connect->security   = network->security;
    connect->key        = g_strdup(key);
    connect->profile_id =
        network->profile_uuid ? atoi(network->profile_uuid) : -1;

    GTask* thread_task =
        g_task_new(device, NULL, on_connect_thread_done, task);

    g_task_set_task_data(
        thread_task,
        connect,
        (GDestroyNotify) net_connect_data_free
    );
    g_task_run_in_thread(thread_task, net_connect_thread);
    g_object_unref(thread_task);
}

void wintc_net_device_disconnect_async(
    WinTCNetDevice*     device,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask* task = g_task_new(device, NULL, callback, user_data);

    if (!(device->wpa_monitor))
    {
        g_task_return_new_error(
            task,
            WINTC_NET_ERROR,
            WINTC_NET_ERROR_FAILED,
            "wpa_supplicant is not running on %s.",
            device->iface
        );
        g_object_unref(task);
        return;
    }

    // wpa_supplicant stays disconnected until told to connect again
    //
    GTask* thread_task =
        g_task_new(device, NULL, on_simple_wpa_done, task);

    g_task_set_task_data(
        thread_task,
        g_strdupv((gchar**) (const gchar*[]) { "DISCONNECT", NULL }),
        (GDestroyNotify) g_strfreev
    );
    g_task_run_in_thread(thread_task, net_run_wpa_thread);
    g_object_unref(thread_task);
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

    data->network_id  = -1;
    data->cancellable = cancellable ? g_object_ref(cancellable) : NULL;

    g_task_set_check_cancellable(task, FALSE);
    g_task_set_task_data(
        task,
        data,
        (GDestroyNotify) net_action_data_free
    );

    // Wireless - reassociating takes the link down and up again, which
    // makes dhclient ask for the lease again
    //
    if (device->wpa_monitor)
    {
        GTask* thread_task =
            g_task_new(device, NULL, on_simple_wpa_done, task);

        g_task_set_task_data(
            thread_task,
            g_strdupv((gchar**) (const gchar*[]) { "REASSOCIATE", NULL }),
            (GDestroyNotify) g_strfreev
        );
        g_task_run_in_thread(thread_task, net_run_wpa_thread);
        g_object_unref(thread_task);

        return;
    }

    // Wired - renew the DHCP lease
    //
    net_run_netctl(
        device,
        "renew",
        on_dhclient_restart_done,
        task
    );
}

void wintc_net_device_set_enabled_async(
    WinTCNetDevice*     device,
    gboolean            enabled,
    GAsyncReadyCallback callback,
    gpointer            user_data
)
{
    GTask* task = g_task_new(device, NULL, callback, user_data);

    // Disabling is taking the interface down, the same as ifconfig down -
    // wpa_supplicant and dhclient follow along by themselves
    //
    net_run_netctl(
        device,
        enabled ? "up" : "down",
        on_ifconfig_done,
        task
    );
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
static void wintc_net_device_apply_snapshot(
    WinTCNetDevice*   device,
    WinTCNetSnapshot* snapshot
)
{
    g_clear_pointer(&(device->snapshot), net_snapshot_free);

    device->snapshot = snapshot;

    wintc_net_device_update_state(device);

    // Only tell the networks window when something it shows changed, the
    // list is rebuilt from scratch each time
    //
    if (device->kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        GString* key = g_string_new(NULL);

        g_list_free_full(
            g_steal_pointer(&(device->networks)),
            (GDestroyNotify) wintc_wlan_network_free
        );

        device->networks = g_steal_pointer(&(snapshot->networks));

        // As with NetworkManager, the network only counts as the one in use
        // once there's an address
        //
        if (!wintc_net_device_is_connected(device))
        {
            for (GList* iter = device->networks; iter; iter = iter->next)
            {
                ((WinTCWlanNetwork*) iter->data)->in_use = FALSE;
            }

            device->networks =
                g_list_sort(device->networks, wintc_wlan_network_compare);
        }

        for (GList* iter = device->networks; iter; iter = iter->next)
        {
            WinTCWlanNetwork* network = (WinTCWlanNetwork*) iter->data;

            g_string_append_printf(
                key,
                "%s|%d|%d|%d|%s|%d\n",
                network->ssid,
                network->security,
                network->in_use,
                wintc_wlan_signal_to_bars(network->signal),
                network->profile_uuid ? network->profile_uuid : "",
                network->profile_autoconnect
            );
        }

        if (
            device->update_networks ||
            g_strcmp0(key->str, device->networks_key) != 0
        )
        {
            g_free(device->networks_key);
            device->networks_key = g_string_free(key, FALSE);

            g_signal_emit(
                device,
                wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
                0
            );
        }
        else
        {
            g_string_free(key, TRUE);
        }
    }

    device->update_networks = FALSE;

    wintc_net_device_check_action(device);

    g_signal_emit(
        device,
        wintc_net_device_signals[SIGNAL_STATE_CHANGED],
        0
    );
}

static void wintc_net_device_check_action(
    WinTCNetDevice* device
)
{
    if (!(device->action))
    {
        return;
    }

    WinTCNetActionData* data = g_task_get_task_data(device->action);

    // The association may have finished before we were listening
    //
    if (
        data->network_id >= 0                                     &&
        device->snapshot->wpa_id == data->network_id              &&
        g_strcmp0(device->snapshot->wpa_state, "COMPLETED") == 0
    )
    {
        data->associated = TRUE;
    }

    // Done once the network is back and dhclient got an address (or gave
    // up, as with NetworkManager a link-local only connection counts)
    //
    if (data->associated && wintc_net_device_is_connected(device))
    {
        net_action_complete(device, NULL);
    }
}

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

static void wintc_net_device_update_state(
    WinTCNetDevice* device
)
{
    WinTCNetSnapshot* snapshot  = device->snapshot;
    WinTCNetState     new_state = WINTC_NET_STATE_DISCONNECTED;
    gboolean          wireless  =
        device->kind == WINTC_NET_DEVICE_KIND_WIRELESS;
    gint64            now       = g_get_real_time() / G_USEC_PER_SEC;

    // With wpa_supplicant around, it knows best whether we're associated
    //
    gboolean link       = snapshot->link;
    gboolean connecting = FALSE;

    if (wireless && snapshot->wpa)
    {
        static const gchar* s_connecting[] = {
            "AUTHENTICATING",
            "ASSOCIATING",
            "ASSOCIATED",
            "4WAY_HANDSHAKE",
            "GROUP_HANDSHAKE",
            NULL
        };

        link       = g_strcmp0(snapshot->wpa_state, "COMPLETED") == 0;
        connecting =
            snapshot->wpa_state &&
            g_strv_contains(s_connecting, snapshot->wpa_state);
    }

    // Remember when the link came up for DHCP - when we start up with it
    // already up, dhclient was started by devd at about the same time
    //
    if (!link)
    {
        device->link_since = 0;
    }
    else if (!(device->link_since))
    {
        if (device->state_known)
        {
            device->link_since = now;
        }
        else
        {
            device->link_since =
                snapshot->dhclient_started ?
                    MIN(snapshot->dhclient_started, now) :
                    now - NET_DHCP_TIMEOUT_S;
        }
    }

    g_clear_handle_id(&(device->id_recheck), g_source_remove);

    if (!(snapshot->exists))
    {
        new_state = WINTC_NET_STATE_UNAVAILABLE;
    }
    else if (!(snapshot->up))
    {
        new_state = WINTC_NET_STATE_DISABLED;
    }
    else if (connecting)
    {
        new_state = WINTC_NET_STATE_CONNECTING;
    }
    else if (!link)
    {
        // Wired - the cable is unplugged
        //
        new_state =
            wireless ?
                WINTC_NET_STATE_DISCONNECTED :
                WINTC_NET_STATE_UNAVAILABLE;
    }
    else if (snapshot->routable)
    {
        new_state = WINTC_NET_STATE_CONNECTED;
    }
    else
    {
        // No address yet - give DHCP its time, then it's 'Limited or no
        // connectivity' like XP
        //
        gint64 since   = MAX(device->link_since, snapshot->dhclient_started);
        gint64 elapsed = now - since;
        gint64 limit   =
            snapshot->dhclient_started ?
                NET_DHCP_TIMEOUT_S :
                NET_DHCP_START_S;

        if (elapsed < limit)
        {
            new_state = WINTC_NET_STATE_ACQUIRING;

            device->id_recheck =
                g_timeout_add_seconds(
                    (guint) (limit - elapsed),
                    on_timeout_recheck,
                    device
                );
        }
        else
        {
            new_state = WINTC_NET_STATE_LIMITED;
        }
    }

    // Track how long we have been connected for the status window
    //
    gboolean was_connected = wintc_net_device_is_connected(device);

    device->state = new_state;

    if (wintc_net_device_is_connected(device))
    {
        if (!(device->state_known))
        {
            device->connected_since =
                snapshot->dhclient_started ?
                    MIN(snapshot->dhclient_started, now) :
                    now;
        }
        else if (!was_connected)
        {
            device->connected_since = now;
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

    // Signal strength and speed change without the kernel saying anything
    //
    if (wireless && wintc_net_device_is_connected(device))
    {
        if (!(device->id_wlan_poll))
        {
            device->id_wlan_poll =
                g_timeout_add_seconds(
                    NET_WLAN_POLL_S,
                    on_timeout_wlan_poll,
                    device
                );
        }
    }
    else
    {
        g_clear_handle_id(&(device->id_wlan_poll), g_source_remove);
    }
}

static void wintc_net_device_update_traffic(
    WinTCNetDevice* device
)
{
    WinTCNetTraffic  traffic;
    WinTCNetActivity activity = WINTC_NET_ACTIVITY_NONE;

    if (!net_read_traffic(device->iface, &traffic))
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

static void wintc_net_device_wpa_attach(
    WinTCNetDevice* device
)
{
    GError* error = NULL;

    device->wpa_monitor = wintc_wpa_monitor_new(device->wpa_path, &error);

    if (!(device->wpa_monitor))
    {
        // Not running is fine (no wireless set up yet), but no access is
        // worth saying something about
        //
        if (
            !(device->wpa_warned) &&
            g_error_matches(error, G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED)
        )
        {
            g_warning(
                "net: no access to %s - the user needs to be in the group "
                "set by wintc_net_group in rc.conf (wheel by default)",
                device->wpa_path
            );

            device->wpa_warned = TRUE;
        }

        WINTC_LOG_DEBUG("net: wpa_supplicant: %s", error->message);
        g_clear_error(&error);

        if (!(device->id_wpa_retry))
        {
            device->id_wpa_retry =
                g_timeout_add_seconds(
                    NET_WPA_RETRY_S,
                    on_timeout_wpa_retry,
                    device
                );
        }

        return;
    }

    WINTC_LOG_DEBUG("net: attached to wpa_supplicant on %s", device->iface);

    g_clear_handle_id(&(device->id_wpa_retry), g_source_remove);

    device->sigid_wpa_event =
        g_signal_connect(
            device->wpa_monitor,
            "event",
            G_CALLBACK(on_wpa_monitor_event),
            device
        );
    device->sigid_wpa_closed =
        g_signal_connect(
            device->wpa_monitor,
            "closed",
            G_CALLBACK(on_wpa_monitor_closed),
            device
        );
}

static void wintc_net_device_wpa_detach(
    WinTCNetDevice* device
)
{
    if (!(device->wpa_monitor))
    {
        return;
    }

    g_clear_signal_handler(&(device->sigid_wpa_event),  device->wpa_monitor);
    g_clear_signal_handler(&(device->sigid_wpa_closed), device->wpa_monitor);
    g_clear_object(&(device->wpa_monitor));
}

static void net_action_complete(
    WinTCNetDevice* device,
    GError*         error
)
{
    GTask*              task = g_steal_pointer(&(device->action));
    WinTCNetActionData* data = g_task_get_task_data(task);

    if (data->completed)
    {
        g_clear_error(&error);
        g_object_unref(task);
        return;
    }

    data->completed = TRUE;

    if (data->cancellable)
    {
        g_clear_signal_handler(
            &(data->sigid_cancelled),
            data->cancellable
        );
    }

    g_clear_handle_id(&(data->id_timeout), g_source_remove);
    g_clear_handle_id(&(data->id_cancel),  g_source_remove);

    // Picking a network with SELECT_NETWORK switches the others off, turn
    // them back on so wpa_supplicant can fall back to them later - and
    // remember the network if it worked
    //
    if (data->network_id >= 0 && device->wpa_path)
    {
        gchar* remove =
            g_strdup_printf("REMOVE_NETWORK %d", data->network_id);

        const gchar* commands[4] = { NULL };
        gint         n           = 0;

        if (error && data->created)
        {
            commands[n++] = remove;
        }

        commands[n++] = "ENABLE_NETWORK all";

        if (!error)
        {
            commands[n++] = "SAVE_CONFIG";
        }

        net_run_wpa_commands(device, commands);

        g_free(remove);
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
    g_clear_object(&(data->cancellable));
    g_free(data);
}

static void net_admin_error(
    GSubprocess* proc,
    GError**     error
)
{
    if (geteuid() == 0 || !g_subprocess_get_if_exited(proc))
    {
        return;
    }

    gint status = g_subprocess_get_exit_status(proc);
    if (status == 126 || status == 127)
    {
        g_clear_error(error);
        g_set_error_literal(
            error,
            G_IO_ERROR,
            status == 126 ? G_IO_ERROR_CANCELLED :
                            G_IO_ERROR_PERMISSION_DENIED,
            status == 126 ?
                "Administrator authorization was cancelled." :
                "Administrator authorization failed. Check that system "
                "D-Bus and a graphical polkit agent are running."
        );
    }
}

static void net_action_start(
    WinTCNetDevice* device,
    GTask*          task
)
{
    WinTCNetActionData* data = g_task_get_task_data(task);

    // Only one at a time, the latest wins
    //
    if (device->action)
    {
        net_action_complete(
            device,
            g_error_new_literal(
                G_IO_ERROR,
                G_IO_ERROR_CANCELLED,
                "Superseded by another action."
            )
        );
    }

    device->action = task;

    data->id_timeout =
        g_timeout_add_seconds(
            data->network_id >= 0 || data->associated ?
                NET_CONNECT_TIMEOUT_S :
                NET_REPAIR_TIMEOUT_S,
            on_action_timeout,
            device
        );

    if (data->cancellable)
    {
        data->sigid_cancelled =
            g_signal_connect(
                data->cancellable,
                "cancelled",
                G_CALLBACK(on_action_cancelled),
                device
            );

        if (g_cancellable_is_cancelled(data->cancellable))
        {
            on_action_cancelled(data->cancellable, device);
        }
    }

    wintc_net_device_queue_update(device, TRUE);
}

static void net_connect_data_free(
    WinTCNetConnectData* data
)
{
    if (data->key)
    {
        explicit_bzero(data->key, strlen(data->key));
        g_free(data->key);
    }

    g_free(data->wpa_path);
    g_bytes_unref(data->ssid);
    g_free(data);
}

static void net_connect_thread(
    GTask*   task,
    WINTC_UNUSED(gpointer source_object),
    gpointer task_data,
    WINTC_UNUSED(GCancellable* cancellable)
)
{
    WinTCNetConnectData* data  = (WinTCNetConnectData*) task_data;
    GError*              error = NULL;
    gchar*               cmd;

    data->network_id = data->profile_id;

    // Picking the network we're already on doesn't do anything in
    // wpa_supplicant, so no events would come
    //
    if (data->network_id >= 0 && !(data->key))
    {
        gchar* status =
            wintc_wpa_ctrl_request(data->wpa_path, "STATUS", NULL);
        gchar* state  = status ? net_wpa_get_field(status, "wpa_state") : NULL;
        gchar* id     = status ? net_wpa_get_field(status, "id") : NULL;

        data->already_connected =
            g_strcmp0(state, "COMPLETED") == 0 &&
            id && atoi(id) == data->network_id;

        g_free(status);
        g_free(state);
        g_free(id);

        if (data->already_connected)
        {
            g_task_return_boolean(task, TRUE);
            return;
        }
    }

    if (data->network_id < 0)
    {
        gchar* reply =
            wintc_wpa_ctrl_request(data->wpa_path, "ADD_NETWORK", &error);

        if (!reply)
        {
            g_task_return_error(task, error);
            return;
        }

        if (!g_ascii_isdigit(reply[0]))
        {
            g_free(reply);
            g_task_return_new_error(
                task,
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_FAILED,
                "wpa_supplicant could not add the network."
            );
            return;
        }

        data->network_id = atoi(reply);
        data->created    = TRUE;

        g_free(reply);

        // The SSID goes in as hex so any bytes are fine
        //
        gsize         len  = 0;
        const guchar* ssid = g_bytes_get_data(data->ssid, &len);
        GString*      hex  = g_string_new(NULL);

        for (gsize i = 0; i < len; i++)
        {
            g_string_append_printf(hex, "%02x", ssid[i]);
        }

        cmd =
            g_strdup_printf(
                "SET_NETWORK %d ssid %s",
                data->network_id,
                hex->str
            );

        g_string_free(hex, TRUE);

        gboolean ok = wintc_wpa_ctrl_request_ok(data->wpa_path, cmd, &error);

        g_free(cmd);

        if (!ok)
        {
            g_task_return_error(task, error);
            return;
        }

        const gchar* key_mgmt = "NONE";

        switch (data->security)
        {
            case WINTC_WLAN_SECURITY_OWE:        key_mgmt = "OWE";     break;
            case WINTC_WLAN_SECURITY_WPA:
            case WINTC_WLAN_SECURITY_WPA2:       key_mgmt = "WPA-PSK"; break;
            case WINTC_WLAN_SECURITY_WPA3:       key_mgmt = "SAE";     break;
            case WINTC_WLAN_SECURITY_ENTERPRISE: key_mgmt = "WPA-EAP"; break;
            default:                                                   break;
        }

        cmd =
            g_strdup_printf(
                "SET_NETWORK %d key_mgmt %s",
                data->network_id,
                key_mgmt
            );

        ok = wintc_wpa_ctrl_request_ok(data->wpa_path, cmd, &error);

        g_free(cmd);

        if (!ok)
        {
            g_task_return_new_error(
                task,
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_FAILED,
                "This wpa_supplicant can't connect to %s networks (%s).",
                key_mgmt,
                error->message
            );
            g_clear_error(&error);
            return;
        }

        if (data->security == WINTC_WLAN_SECURITY_WPA3)
        {
            cmd =
                g_strdup_printf(
                    "SET_NETWORK %d ieee80211w 2",
                    data->network_id
                );

            wintc_wpa_ctrl_request_ok(data->wpa_path, cmd, NULL);
            g_free(cmd);
        }
    }

    // The key, if we were given one (new network or the old key is wrong)
    //
    gboolean needs_key =
        data->security == WINTC_WLAN_SECURITY_WEP  ||
        data->security == WINTC_WLAN_SECURITY_WPA  ||
        data->security == WINTC_WLAN_SECURITY_WPA2 ||
        data->security == WINTC_WLAN_SECURITY_WPA3;

    if (needs_key && data->key)
    {
        gchar* value = net_format_key(data->security, data->key);

        if (!value)
        {
            g_task_return_new_error(
                task,
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_SECRETS_REQUIRED,
                "The network key is not valid."
            );
            return;
        }

        const gchar* field = "psk";

        if (data->security == WINTC_WLAN_SECURITY_WEP)
        {
            field = "wep_key0";
        }
        else if (data->security == WINTC_WLAN_SECURITY_WPA3)
        {
            field = "sae_password";
        }

        cmd =
            g_strdup_printf(
                "SET_NETWORK %d %s %s",
                data->network_id,
                field,
                value
            );

        gboolean ok = wintc_wpa_ctrl_request_ok(data->wpa_path, cmd, NULL);

        explicit_bzero(cmd, strlen(cmd));
        explicit_bzero(value, strlen(value));
        g_free(cmd);
        g_free(value);

        if (!ok)
        {
            g_task_return_new_error(
                task,
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_SECRETS_REQUIRED,
                "The network key is not valid."
            );
            return;
        }
    }
    else if (needs_key && data->created)
    {
        g_task_return_new_error(
            task,
            WINTC_NET_ERROR,
            WINTC_NET_ERROR_SECRETS_REQUIRED,
            "The network needs a key."
        );
        return;
    }

    cmd = g_strdup_printf("SELECT_NETWORK %d", data->network_id);

    gboolean ok = wintc_wpa_ctrl_request_ok(data->wpa_path, cmd, &error);

    g_free(cmd);

    if (!ok)
    {
        g_task_return_error(task, error);
        return;
    }

    g_task_return_boolean(task, TRUE);
}

static gboolean net_dhclient_running(
    const gchar* iface,
    gint64*      started
)
{
    gchar*   path     = g_strdup_printf(NET_DHCLIENT_PID, iface);
    gchar*   contents = NULL;
    gboolean running  = FALSE;

    if (g_file_get_contents(path, &contents, NULL, NULL))
    {
        pid_t pid = (pid_t) g_ascii_strtoll(contents, NULL, 10);

        // It runs as root, so EPERM still means it's there
        //
        running = pid > 0 && (kill(pid, 0) == 0 || errno == EPERM);

        GStatBuf st;

        if (running && started && g_stat(path, &st) == 0)
        {
            *started = (gint64) st.st_mtime;
        }
    }

    g_free(contents);
    g_free(path);

    return running;
}

static gchar* net_format_key(
    WinTCWlanSecurity security,
    const gchar*      key
)
{
    gsize    len = strlen(key);
    gboolean hex = len > 0;

    for (gsize i = 0; i < len; i++)
    {
        // A newline would end the command early
        //
        if (key[i] == '\n' || key[i] == '\r')
        {
            return NULL;
        }

        hex = hex && g_ascii_isxdigit(key[i]);
    }

    // Raw keys go in as hex, passphrases and ASCII keys in quotes
    //
    if (security == WINTC_WLAN_SECURITY_WEP)
    {
        if (hex && (len == 10 || len == 26))
        {
            return g_strdup(key);
        }

        return len == 5 || len == 13 ? g_strdup_printf("\"%s\"", key) : NULL;
    }

    if (security != WINTC_WLAN_SECURITY_WPA3 && hex && len == 64)
    {
        return g_strdup(key);
    }

    if (security != WINTC_WLAN_SECURITY_WPA3 && (len < 8 || len > 63))
    {
        return NULL;
    }

    return g_strdup_printf("\"%s\"", key);
}

static gchar* net_get_gateway(
    guint ifindex
)
{
    gint   mib[6]  = { CTL_NET, PF_ROUTE, 0, AF_INET, NET_RT_DUMP, 0 };
    gsize  len     = 0;
    gchar* gateway = NULL;

    if (!ifindex || sysctl(mib, 6, NULL, &len, NULL, 0) < 0)
    {
        return NULL;
    }

    gchar* buf = g_malloc(len);

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0)
    {
        g_free(buf);
        return NULL;
    }

    for (
        gchar* next = buf;
        next < buf + len && !gateway;
        next += ((struct rt_msghdr*) next)->rtm_msglen
    )
    {
        struct rt_msghdr* rtm = (struct rt_msghdr*) next;
        struct sockaddr*  rti_info[RTAX_MAX] = { 0 };
        struct sockaddr*  sa  = (struct sockaddr*) (rtm + 1);

        if (!(rtm->rtm_msglen))
        {
            break;
        }

        if (rtm->rtm_index != ifindex || !(rtm->rtm_flags & RTF_GATEWAY))
        {
            continue;
        }

        for (gint i = 0; i < RTAX_MAX; i++)
        {
            if (rtm->rtm_addrs & (1 << i))
            {
                rti_info[i] = sa;
                sa = (struct sockaddr*) ((gchar*) sa + SA_SIZE(sa));
            }
        }

        // The default route is the one to 0.0.0.0
        //
        struct sockaddr* dst = rti_info[RTAX_DST];
        struct sockaddr* gw  = rti_info[RTAX_GATEWAY];

        if (
            dst && dst->sa_family == AF_INET &&
            ((struct sockaddr_in*) dst)->sin_addr.s_addr == INADDR_ANY &&
            gw && gw->sa_family == AF_INET
        )
        {
            gchar addr[INET_ADDRSTRLEN];

            inet_ntop(
                AF_INET,
                &(((struct sockaddr_in*) gw)->sin_addr),
                addr,
                sizeof(addr)
            );

            gateway = g_strdup(addr);
        }
    }

    g_free(buf);

    return gateway;
}

static void net_read_lease(
    const gchar*    iface,
    WinTCNetIpInfo* info
)
{
    // dhclient only lets root read the leases, without them the status
    // window just doesn't show the lease times
    //
    gchar* path     = g_strdup_printf(NET_DHCLIENT_LEASE, iface);
    gchar* contents = NULL;

    if (!g_file_get_contents(path, &contents, NULL, NULL))
    {
        g_free(path);
        return;
    }

    // The newest lease is the last one in the file
    //
    gchar* lease = g_strrstr(contents, "lease {");

    if (lease)
    {
        gchar** lines      = g_strsplit(lease, "\n", -1);
        gint64  lease_time = 0;

        for (gchar** line = lines; *line; line++)
        {
            gchar* text = g_strstrip(*line);
            gint   y, mo, d, h, mi, s;

            g_strdelimit(text, ";", ' ');
            g_strchomp(text);

            if (g_str_has_prefix(text, "option dhcp-server-identifier "))
            {
                g_free(info->dhcp_server);
                info->dhcp_server = g_strdup(text + 30);
            }
            else if (g_str_has_prefix(text, "option dhcp-lease-time "))
            {
                lease_time = g_ascii_strtoll(text + 23, NULL, 10);
            }
            else if (
                sscanf(
                    text,
                    "expire %*d %d/%d/%d %d:%d:%d",
                    &y, &mo, &d, &h, &mi, &s
                ) == 6
            )
            {
                GDateTime* expires =
                    g_date_time_new_utc(y, mo, d, h, mi, (gdouble) s);

                if (expires)
                {
                    info->lease_expires = g_date_time_to_unix(expires);
                    g_date_time_unref(expires);
                }
            }
        }

        if (info->lease_expires && lease_time)
        {
            info->lease_obtained = info->lease_expires - lease_time;
        }

        g_strfreev(lines);
    }

    g_free(contents);
    g_free(path);
}

static gboolean net_read_traffic(
    const gchar*     iface,
    WinTCNetTraffic* traffic
)
{
    struct ifaddrs* ifap  = NULL;
    gboolean        found = FALSE;

    if (getifaddrs(&ifap) != 0)
    {
        return FALSE;
    }

    for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next)
    {
        if (
            !(ifa->ifa_addr)                    ||
            ifa->ifa_addr->sa_family != AF_LINK ||
            !(ifa->ifa_data)                    ||
            g_strcmp0(ifa->ifa_name, iface) != 0
        )
        {
            continue;
        }

        struct if_data* data = (struct if_data*) ifa->ifa_data;

        traffic->tx_bytes   = data->ifi_obytes;
        traffic->rx_bytes   = data->ifi_ibytes;
        traffic->tx_packets = data->ifi_opackets;
        traffic->rx_packets = data->ifi_ipackets;

        found = TRUE;
        break;
    }

    freeifaddrs(ifap);

    return found;
}

static void net_read_wireless(
    gint              sock,
    const gchar*      iface,
    WinTCNetSnapshot* snapshot
)
{
    // Ask net80211 about the AP we're associated with, for the current
    // rate and signal
    //
    struct ieee80211req ireq = { 0 };
    guint8              bssid[IEEE80211_ADDR_LEN];

    g_strlcpy(ireq.i_name, iface, sizeof(ireq.i_name));

    ireq.i_type = IEEE80211_IOC_BSSID;
    ireq.i_data = bssid;
    ireq.i_len  = sizeof(bssid);

    if (ioctl(sock, SIOCG80211, &ireq) < 0)
    {
        return;
    }

    union
    {
        struct ieee80211req_sta_req req;
        guint8                      buf[4096];
    } sta;

    memset(&sta, 0, sizeof(sta));
    memcpy(sta.req.is_u.macaddr, bssid, sizeof(bssid));

    ireq.i_type = IEEE80211_IOC_STA_INFO;
    ireq.i_data = &sta;
    ireq.i_len  = sizeof(sta);

    if (ioctl(sock, SIOCG80211, &ireq) < 0 || ireq.i_len < sizeof(sta.req))
    {
        return;
    }

    struct ieee80211req_sta_info* info = &(sta.req.info[0]);

    if (info->isi_txmbps)
    {
        snapshot->speed = (guint32) info->isi_txmbps * 500;
    }

    snapshot->signal =
        net_signal_from_dbm(info->isi_rssi / 2 + info->isi_noise);
}

static void net_read_wpa(
    const gchar*      wpa_path,
    WinTCNetSnapshot* snapshot
)
{
    gchar* status = wintc_wpa_ctrl_request(wpa_path, "STATUS", NULL);

    if (!status)
    {
        return;
    }

    gchar* scan     = wintc_wpa_ctrl_request(wpa_path, "SCAN_RESULTS", NULL);
    gchar* profiles = wintc_wpa_ctrl_request(wpa_path, "LIST_NETWORKS", NULL);

    gchar* id = net_wpa_get_field(status, "id");

    snapshot->wpa       = TRUE;
    snapshot->wpa_state = net_wpa_get_field(status, "wpa_state");
    snapshot->wpa_id    = id ? atoi(id) : -1;

    g_free(id);

    gboolean completed = g_strcmp0(snapshot->wpa_state, "COMPLETED") == 0;
    gchar*   cur_text  = net_wpa_get_field(status, "ssid");
    GBytes*  cur_ssid  =
        completed && cur_text ? net_wpa_decode_text(cur_text) : NULL;

    // XP lists networks, not access points - pick the best AP for each SSID
    //
    GHashTable* map_networks = g_hash_table_new(g_str_hash, g_str_equal);
    gchar**     lines        = g_strsplit(scan ? scan : "", "\n", -1);

    for (gchar** line = lines; *line && *(line + 1); line++)
    {
        // bssid / frequency / signal level / flags / ssid - the last line
        // may be cut short if the reply was too big, so it's skipped
        //
        gchar** fields = g_strsplit(*line, "\t", 5);

        if (g_strv_length(fields) != 5 || !*(fields[4]))
        {
            g_strfreev(fields);
            continue; // FIXME: Hidden networks are not listed
        }

        GBytes* ssid_raw  = net_wpa_decode_text(fields[4]);
        gchar*  ssid      =
            g_utf8_make_valid(
                g_bytes_get_data(ssid_raw, NULL),
                g_bytes_get_size(ssid_raw)
            );
        gint    strength  =
            net_signal_from_dbm((gint) g_ascii_strtoll(fields[2], NULL, 10));
        gboolean is_active =
            cur_ssid && g_bytes_equal(cur_ssid, ssid_raw);

        WinTCWlanNetwork* network = g_hash_table_lookup(map_networks, ssid);

        if (!network)
        {
            network = g_new0(WinTCWlanNetwork, 1);

            network->ssid   = ssid;
            network->signal = -1;

            g_hash_table_insert(map_networks, network->ssid, network);

            snapshot->networks = g_list_prepend(snapshot->networks, network);
        }
        else
        {
            g_free(ssid);
        }

        if (strength > network->signal)
        {
            g_free(network->ap_path);
            g_clear_pointer(&(network->ssid_raw), g_bytes_unref);

            network->ap_path  = g_strdup(fields[0]);
            network->ssid_raw = g_bytes_ref(ssid_raw);
            network->signal   = strength;
            network->security = net_wpa_flags_to_security(fields[3]);
        }

        network->in_use = network->in_use || is_active;

        g_bytes_unref(ssid_raw);
        g_strfreev(fields);
    }

    g_strfreev(lines);

    // The network we're on might have dropped out of the scan results
    //
    if (cur_ssid)
    {
        gchar* ssid =
            g_utf8_make_valid(
                g_bytes_get_data(cur_ssid, NULL),
                g_bytes_get_size(cur_ssid)
            );

        WinTCWlanNetwork* network = g_hash_table_lookup(map_networks, ssid);

        if (!network)
        {
            gchar* key_mgmt = net_wpa_get_field(status, "key_mgmt");
            gchar* cipher   = net_wpa_get_field(status, "pairwise_cipher");

            network = g_new0(WinTCWlanNetwork, 1);

            network->ssid     = g_steal_pointer(&ssid);
            network->ssid_raw = g_bytes_ref(cur_ssid);
            network->ap_path  = net_wpa_get_field(status, "bssid");
            network->signal   = 0;
            network->in_use   = TRUE;
            network->security =
                g_str_has_prefix(cipher ? cipher : "", "WEP") ?
                    WINTC_WLAN_SECURITY_WEP :
                    net_wpa_flags_to_security(key_mgmt ? key_mgmt : "");

            snapshot->networks = g_list_prepend(snapshot->networks, network);

            g_free(key_mgmt);
            g_free(cipher);
        }

        // The live signal is better than what the last scan said
        //
        if (snapshot->signal >= 0)
        {
            network->signal = snapshot->signal;
        }

        g_free(ssid);
    }

    g_hash_table_unref(map_networks);

    // Look up the preferred networks - the ones wpa_supplicant knows
    //
    lines = g_strsplit(profiles ? profiles : "", "\n", -1);

    for (gchar** line = lines; *line; line++)
    {
        // network id / ssid / bssid / flags
        //
        gchar** fields = g_strsplit(*line, "\t", 4);

        if (g_strv_length(fields) < 3 || !g_ascii_isdigit(fields[0][0]))
        {
            g_strfreev(fields);
            continue;
        }

        GBytes* ssid_raw = net_wpa_decode_text(fields[1]);

        for (GList* iter = snapshot->networks; iter; iter = iter->next)
        {
            WinTCWlanNetwork* network = (WinTCWlanNetwork*) iter->data;

            if (
                !(network->profile_uuid) &&
                g_bytes_equal(network->ssid_raw, ssid_raw)
            )
            {
                network->profile_uuid        = g_strdup(fields[0]);
                network->profile_autoconnect =
                    !fields[3] || !strstr(fields[3], "[DISABLED]");
            }
        }

        g_bytes_unref(ssid_raw);
        g_strfreev(fields);
    }

    g_strfreev(lines);

    snapshot->networks =
        g_list_sort(snapshot->networks, wintc_wlan_network_compare);

    g_clear_pointer(&cur_ssid, g_bytes_unref);
    g_free(cur_text);
    g_free(status);
    g_free(scan);
    g_free(profiles);
}

static void net_refresh_data_free(
    WinTCNetRefreshData* data
)
{
    g_free(data->iface);
    g_free(data->wpa_path);
    g_free(data);
}

static void net_refresh_thread(
    GTask*   task,
    WINTC_UNUSED(gpointer source_object),
    gpointer task_data,
    WINTC_UNUSED(GCancellable* cancellable)
)
{
    g_task_return_pointer(
        task,
        net_snapshot_take((WinTCNetRefreshData*) task_data),
        (GDestroyNotify) net_snapshot_free
    );
}

static void net_run_netctl(
    WinTCNetDevice*     device,
    const gchar*        verb,
    GAsyncReadyCallback callback,
    GTask*              task
)
{
    // Changing adapters needs root, the helper is run through pkexec(1) -
    // the polkit policy that comes with it lets the user in the active
    // session do it without a password, like NetworkManager does
    //
    const gchar* argv[] = {
        NET_PKEXEC_PATH,
        "--disable-internal-agent",
        WINTC_NETCTL_PATH,
        verb,
        device->iface,
        NULL
    };

    GError*      error = NULL;
    GSubprocess* proc  =
        g_subprocess_newv(
            geteuid() == 0 ? argv + 2 : argv,
            G_SUBPROCESS_FLAGS_STDOUT_SILENCE |
            G_SUBPROCESS_FLAGS_STDERR_SILENCE,
            &error
        );

    if (!proc)
    {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }

    g_subprocess_wait_check_async(
        proc,
        NULL,
        callback,
        task
    );

    g_object_unref(proc);
}

static void net_run_wpa_commands(
    WinTCNetDevice*     device,
    const gchar* const* commands
)
{
    // Nobody waits for these, but what they change should show
    //
    GTask* task =
        g_task_new(device, device->cancellable, on_wpa_commands_done, NULL);

    g_task_set_task_data(
        task,
        g_strdupv((gchar**) commands),
        (GDestroyNotify) g_strfreev
    );
    g_task_run_in_thread(task, net_run_wpa_thread);
    g_object_unref(task);
}

static void net_run_wpa_thread(
    GTask*   task,
    gpointer source_object,
    gpointer task_data,
    WINTC_UNUSED(GCancellable* cancellable)
)
{
    WinTCNetDevice* device   = WINTC_NET_DEVICE(source_object);
    gchar**         commands = (gchar**) task_data;
    GError*         error    = NULL;

    // The path never changes, it's fine to read from here
    //
    for (gchar** cmd = commands; *cmd; cmd++)
    {
        if (!wintc_wpa_ctrl_request_ok(device->wpa_path, *cmd, &error))
        {
            g_task_return_error(task, error);
            return;
        }
    }

    g_task_return_boolean(task, TRUE);
}

static gint net_signal_from_dbm(
    gint dbm
)
{
    // The same scale NetworkManager uses, -100 dBm is nothing and -50 dBm
    // is full strength
    //
    return CLAMP(2 * (dbm + 100), 0, 100);
}

static void net_snapshot_free(
    WinTCNetSnapshot* snapshot
)
{
    g_list_free_full(
        snapshot->networks,
        (GDestroyNotify) wintc_wlan_network_free
    );

    g_free(snapshot->wpa_state);
    g_free(snapshot);
}

static WinTCNetSnapshot* net_snapshot_take(
    WinTCNetRefreshData* data
)
{
    WinTCNetSnapshot* snapshot  = g_new0(WinTCNetSnapshot, 1);
    struct ifaddrs*   ifap      = NULL;
    gboolean          link_up   = FALSE;
    guint64           baudrate  = 0;

    snapshot->signal = -1;
    snapshot->wpa_id = -1;

    if (getifaddrs(&ifap) == 0)
    {
        for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            if (!(ifa->ifa_addr) || g_strcmp0(ifa->ifa_name, data->iface))
            {
                continue;
            }

            switch (ifa->ifa_addr->sa_family)
            {
                case AF_LINK:
                {
                    struct if_data* ifd = (struct if_data*) ifa->ifa_data;

                    snapshot->exists = TRUE;
                    snapshot->up     = (ifa->ifa_flags & IFF_UP) != 0;

                    if (ifd)
                    {
                        link_up  = ifd->ifi_link_state == LINK_STATE_UP;
                        baudrate = ifd->ifi_baudrate;
                    }

                    break;
                }

                // A link-local address alone is what XP calls limited
                // connectivity
                //
                case AF_INET:
                {
                    in_addr_t addr =
                        ntohl(
                            ((struct sockaddr_in*) ifa->ifa_addr)
                                ->sin_addr.s_addr
                        );

                    if ((addr & 0xffff0000) != 0xa9fe0000)
                    {
                        snapshot->routable = TRUE;
                    }

                    break;
                }

                case AF_INET6:
                {
                    struct in6_addr* addr =
                        &(((struct sockaddr_in6*) ifa->ifa_addr)->sin6_addr);

                    if (
                        !IN6_IS_ADDR_LINKLOCAL(addr) &&
                        !IN6_IS_ADDR_LOOPBACK(addr)
                    )
                    {
                        snapshot->routable = TRUE;
                    }

                    break;
                }

                default:
                    break;
            }
        }

        freeifaddrs(ifap);
    }

    if (!(snapshot->exists))
    {
        return snapshot;
    }

    // The media status is the cable (or association) - drivers without it
    // only have the link state
    //
    gint              sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct ifmediareq ifmr = { 0 };

    snapshot->link = link_up;

    g_strlcpy(ifmr.ifm_name, data->iface, sizeof(ifmr.ifm_name));

    if (
        sock >= 0 &&
        (
            ioctl(sock, SIOCGIFXMEDIA, &ifmr) == 0 ||
            ioctl(sock, SIOCGIFMEDIA,  &ifmr) == 0
        )
    )
    {
        static const struct ifmedia_baudrate s_rates[] =
            IFM_BAUDRATE_DESCRIPTIONS;

        if (ifmr.ifm_status & IFM_AVALID)
        {
            snapshot->link = (ifmr.ifm_status & IFM_ACTIVE) != 0;
        }

        for (gsize i = 0; s_rates[i].ifmb_word; i++)
        {
            if (IFM_TYPE_MATCH(s_rates[i].ifmb_word, ifmr.ifm_active))
            {
                baudrate = s_rates[i].ifmb_baudrate;
                break;
            }
        }
    }

    snapshot->speed = (guint32) MIN(baudrate / 1000, G_MAXUINT32);

    if (data->wireless && sock >= 0 && snapshot->link)
    {
        net_read_wireless(sock, data->iface, snapshot);
    }

    if (sock >= 0)
    {
        close(sock);
    }

    net_dhclient_running(data->iface, &(snapshot->dhclient_started));

    if (data->wpa_path)
    {
        net_read_wpa(data->wpa_path, snapshot);
    }

    return snapshot;
}

static GBytes* net_wpa_decode_text(
    const gchar* text
)
{
    // wpa_supplicant escapes SSIDs like a C string (printf_encode())
    //
    GByteArray* bytes = g_byte_array_new();

    for (const gchar* p = text; *p; p++)
    {
        guint8 c = (guint8) *p;

        if (c == '\\' && *(p + 1))
        {
            p++;

            switch (*p)
            {
                case 'n': c = '\n';   break;
                case 'r': c = '\r';   break;
                case 't': c = '\t';   break;
                case 'e': c = '\033'; break;

                case 'x':
                    if (
                        g_ascii_isxdigit(*(p + 1)) &&
                        g_ascii_isxdigit(*(p + 2))
                    )
                    {
                        c = (guint8) (
                            (g_ascii_xdigit_value(*(p + 1)) << 4) |
                            g_ascii_xdigit_value(*(p + 2))
                        );
                        p += 2;
                    }
                    break;

                default:
                    c = (guint8) *p;
                    break;
            }
        }

        g_byte_array_append(bytes, &c, 1);
    }

    return g_byte_array_free_to_bytes(bytes);
}

static WinTCWlanSecurity net_wpa_flags_to_security(
    const gchar* flags
)
{
    // Scan results look like [WPA2-PSK-CCMP][ESS], STATUS has the key
    // management on its own (WPA2-PSK)
    //
    if (strstr(flags, "EAP"))
    {
        return WINTC_WLAN_SECURITY_ENTERPRISE;
    }

    if (strstr(flags, "WPA2") || strstr(flags, "RSN"))
    {
        if (strstr(flags, "PSK"))
        {
            return WINTC_WLAN_SECURITY_WPA2;
        }

        if (strstr(flags, "SAE"))
        {
            return WINTC_WLAN_SECURITY_WPA3;
        }

        if (strstr(flags, "OWE"))
        {
            return WINTC_WLAN_SECURITY_OWE;
        }

        return WINTC_WLAN_SECURITY_WPA2;
    }

    if (strstr(flags, "WPA"))
    {
        return WINTC_WLAN_SECURITY_WPA;
    }

    if (strstr(flags, "SAE"))
    {
        return WINTC_WLAN_SECURITY_WPA3;
    }

    if (strstr(flags, "WEP"))
    {
        return WINTC_WLAN_SECURITY_WEP;
    }

    return WINTC_WLAN_SECURITY_NONE;
}

static gchar* net_wpa_get_field(
    const gchar* reply,
    const gchar* key
)
{
    gchar** lines = g_strsplit(reply, "\n", -1);
    gchar*  value = NULL;
    gsize   len   = strlen(key);

    for (gchar** line = lines; *line && !value; line++)
    {
        if (strncmp(*line, key, len) == 0 && (*line)[len] == '=')
        {
            value = g_strdup(*line + len + 1);
        }
    }

    g_strfreev(lines);

    return value;
}

//
// CALLBACKS
//
static void on_action_cancelled(
    WINTC_UNUSED(GCancellable* cancellable),
    gpointer user_data
)
{
    WinTCNetDevice*     device = WINTC_NET_DEVICE(user_data);
    WinTCNetActionData* data   = g_task_get_task_data(device->action);

    // Can't complete the task from inside the cancelled signal, the handler
    // would be disconnected during its own emission
    //
    if (!(data->id_cancel))
    {
        data->id_cancel = g_idle_add(on_action_cancelled_idle, device);
    }
}

static gboolean on_action_cancelled_idle(
    gpointer user_data
)
{
    WinTCNetDevice*     device = WINTC_NET_DEVICE(user_data);
    WinTCNetActionData* data   = g_task_get_task_data(device->action);

    data->id_cancel = 0;

    net_action_complete(
        device,
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
    WinTCNetDevice*     device = WINTC_NET_DEVICE(user_data);
    WinTCNetActionData* data   = g_task_get_task_data(device->action);

    data->id_timeout = 0;

    net_action_complete(
        device,
        g_error_new_literal(
            G_IO_ERROR,
            G_IO_ERROR_TIMED_OUT,
            "Timed out waiting for the connection."
        )
    );

    return G_SOURCE_REMOVE;
}

static void on_connect_thread_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNetDevice*      device  = WINTC_NET_DEVICE(source_object);
    GTask*               task    = G_TASK(user_data);
    WinTCNetActionData*  data    = g_task_get_task_data(task);
    WinTCNetConnectData* connect = g_task_get_task_data(G_TASK(res));
    GError*              error   = NULL;

    if (!g_task_propagate_boolean(G_TASK(res), &error))
    {
        // Don't leave a half set up network behind
        //
        if (connect->created && connect->network_id >= 0)
        {
            gchar* remove =
                g_strdup_printf("REMOVE_NETWORK %d", connect->network_id);

            net_run_wpa_commands(
                device,
                (const gchar*[]) { remove, NULL }
            );

            g_free(remove);
        }

        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }

    if (connect->already_connected)
    {
        g_task_return_boolean(task, TRUE);
        g_object_unref(task);
        return;
    }

    if (g_cancellable_is_cancelled(device->cancellable))
    {
        g_task_return_new_error(
            task,
            G_IO_ERROR,
            G_IO_ERROR_CANCELLED,
            "The adapter went away."
        );
        g_object_unref(task);
        return;
    }

    data->network_id = connect->network_id;
    data->created    = connect->created;

    net_action_start(device, task);
}

static void on_dhclient_restart_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*          task   = G_TASK(user_data);
    WinTCNetDevice* device =
        WINTC_NET_DEVICE(g_task_get_source_object(task));
    GError*         error  = NULL;

    if (
        !g_subprocess_wait_check_finish(
            G_SUBPROCESS(source_object),
            res,
            &error
        )
    )
    {
        net_admin_error(G_SUBPROCESS(source_object), &error);
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }

    // dhclient only goes into the background once it has a lease (or gave
    // up), wait for the address to show
    //
    WinTCNetActionData* data = g_task_get_task_data(task);

    data->associated = TRUE;

    net_action_start(device, task);
}

static void on_ifconfig_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GTask*          task   = G_TASK(user_data);
    WinTCNetDevice* device =
        WINTC_NET_DEVICE(g_task_get_source_object(task));
    GError*         error  = NULL;

    if (!g_subprocess_wait_check_finish(
            G_SUBPROCESS(source_object), res, &error))
    {
        net_admin_error(G_SUBPROCESS(source_object), &error);
        g_task_return_error(task, error);
    }
    else
    {
        g_task_return_boolean(task, TRUE);
        wintc_net_device_queue_update(device, TRUE);
    }

    g_object_unref(task);
}

static void on_refresh_done(
    GObject*      source_object,
    GAsyncResult* res,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCNetDevice*   device   = WINTC_NET_DEVICE(source_object);
    WinTCNetSnapshot* snapshot =
        g_task_propagate_pointer(G_TASK(res), NULL);

    if (!snapshot)
    {
        return;
    }

    if (g_cancellable_is_cancelled(device->cancellable))
    {
        net_snapshot_free(snapshot);
        return;
    }

    device->refreshing = FALSE;

    wintc_net_device_apply_snapshot(device, snapshot);

    if (device->refresh_queued)
    {
        device->refresh_queued = FALSE;

        wintc_net_device_queue_update(device, FALSE);
    }
}

static void on_scan_request_done(
    GObject*      source_object,
    GAsyncResult* res,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(source_object);
    GError*         error  = NULL;

    if (g_task_propagate_boolean(G_TASK(res), &error))
    {
        return;
    }

    // FAIL-BUSY means a scan is already going, the results will come
    //
    if (
        g_cancellable_is_cancelled(device->cancellable) ||
        strstr(error->message, "FAIL-BUSY")
    )
    {
        g_clear_error(&error);
        return;
    }

    WINTC_LOG_DEBUG("net: scan request failed: %s", error->message);
    g_clear_error(&error);

    g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);

    device->scanning = FALSE;

    g_signal_emit(
        device,
        wintc_net_device_signals[SIGNAL_NETWORKS_CHANGED],
        0
    );
}

static void on_simple_wpa_done(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(source_object);
    GTask*          task   = G_TASK(user_data);
    GError*         error  = NULL;

    if (!g_task_propagate_boolean(G_TASK(res), &error))
    {
        g_task_return_error(task, error);
        g_object_unref(task);
        return;
    }

    // Repairs wait for the network to come back, everything else is done
    //
    WinTCNetActionData* data = g_task_get_task_data(task);

    if (data && !g_cancellable_is_cancelled(device->cancellable))
    {
        net_action_start(device, task);
        return;
    }

    wintc_net_device_queue_update(device, TRUE);

    g_task_return_boolean(task, TRUE);
    g_object_unref(task);
}

static gboolean on_timeout_activity(
    gpointer user_data
)
{
    wintc_net_device_update_traffic(WINTC_NET_DEVICE(user_data));

    return G_SOURCE_CONTINUE;
}

static gboolean on_timeout_recheck(
    gpointer user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    device->id_recheck = 0;

    wintc_net_device_queue_update(device, FALSE);

    return G_SOURCE_REMOVE;
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

    device->id_update = 0;

    if (device->refreshing)
    {
        device->refresh_queued = TRUE;
        return G_SOURCE_REMOVE;
    }

    device->refreshing = TRUE;

    WinTCNetRefreshData* data = g_new0(WinTCNetRefreshData, 1);

    data->iface    = g_strdup(device->iface);
    data->wireless = device->kind == WINTC_NET_DEVICE_KIND_WIRELESS;
    data->wpa_path =
        device->wpa_monitor ? g_strdup(device->wpa_path) : NULL;

    GTask* task =
        g_task_new(device, device->cancellable, on_refresh_done, NULL);

    g_task_set_task_data(
        task,
        data,
        (GDestroyNotify) net_refresh_data_free
    );
    g_task_run_in_thread(task, net_refresh_thread);
    g_object_unref(task);

    return G_SOURCE_REMOVE;
}

static gboolean on_timeout_wlan_poll(
    gpointer user_data
)
{
    wintc_net_device_queue_update(WINTC_NET_DEVICE(user_data), FALSE);

    return G_SOURCE_CONTINUE;
}

static gboolean on_timeout_wpa_retry(
    gpointer user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    device->id_wpa_retry = 0;

    wintc_net_device_wpa_attach(device);

    if (device->wpa_monitor)
    {
        wintc_net_device_queue_update(device, TRUE);
    }

    return G_SOURCE_REMOVE;
}

static void on_wpa_commands_done(
    GObject*      source_object,
    GAsyncResult* res,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(source_object);
    GError*         error  = NULL;

    if (!g_task_propagate_boolean(G_TASK(res), &error))
    {
        if (!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
        {
            WINTC_LOG_DEBUG("net: wpa_supplicant: %s", error->message);
        }

        g_clear_error(&error);
    }

    if (!g_cancellable_is_cancelled(device->cancellable))
    {
        wintc_net_device_queue_update(device, TRUE);
    }
}

static void on_wpa_monitor_closed(
    WINTC_UNUSED(WinTCWpaMonitor* monitor),
    gpointer user_data
)
{
    WinTCNetDevice* device = WINTC_NET_DEVICE(user_data);

    WINTC_LOG_DEBUG("net: lost wpa_supplicant on %s", device->iface);

    wintc_net_device_wpa_detach(device);

    if (device->action)
    {
        net_action_complete(
            device,
            g_error_new_literal(
                WINTC_NET_ERROR,
                WINTC_NET_ERROR_FAILED,
                "wpa_supplicant went away."
            )
        );
    }

    device->scanning = FALSE;

    g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);

    if (!(device->id_wpa_retry))
    {
        device->id_wpa_retry =
            g_timeout_add_seconds(
                NET_WPA_RETRY_S,
                on_timeout_wpa_retry,
                device
            );
    }

    wintc_net_device_queue_update(device, TRUE);
}

static void on_wpa_monitor_event(
    WINTC_UNUSED(WinTCWpaMonitor* monitor),
    const gchar* text,
    gpointer     user_data
)
{
    WinTCNetDevice*     device = WINTC_NET_DEVICE(user_data);
    WinTCNetActionData* data   =
        device->action ? g_task_get_task_data(device->action) : NULL;

    WINTC_LOG_DEBUG("net: wpa_supplicant: %s", text);

    if (g_str_has_prefix(text, "CTRL-EVENT-SCAN-RESULTS"))
    {
        g_clear_handle_id(&(device->id_scan_timeout), g_source_remove);

        device->scanning = FALSE;

        wintc_net_device_queue_update(device, TRUE);
    }
    else if (g_str_has_prefix(text, "CTRL-EVENT-CONNECTED"))
    {
        // ...completed [id=0 id_str=]
        //
        const gchar* id = strstr(text, "[id=");

        if (
            data &&
            (data->network_id < 0 || (id && atoi(id + 4) == data->network_id))
        )
        {
            data->associated = TRUE;
        }

        wintc_net_device_queue_update(device, TRUE);
    }
    else if (g_str_has_prefix(text, "CTRL-EVENT-SSID-TEMP-DISABLED"))
    {
        // id=0 ssid="..." auth_failures=1 duration=10 reason=WRONG_KEY
        //
        const gchar* id = strstr(text, "id=");

        if (data && id && atoi(id + 3) == data->network_id)
        {
            gboolean wrong_key = strstr(text, "reason=WRONG_KEY") != NULL;

            net_action_complete(
                device,
                g_error_new_literal(
                    WINTC_NET_ERROR,
                    wrong_key ?
                        WINTC_NET_ERROR_SECRETS_REQUIRED :
                        WINTC_NET_ERROR_FAILED,
                    wrong_key ?
                        "The network key is missing or incorrect." :
                        "The network refused the connection."
                )
            );
        }

        wintc_net_device_queue_update(device, TRUE);
    }
    else if (g_str_has_prefix(text, "CTRL-EVENT-TERMINATING"))
    {
        on_wpa_monitor_closed(device->wpa_monitor, device);
    }
    else if (
        g_str_has_prefix(text, "CTRL-EVENT-DISCONNECTED") ||
        g_str_has_prefix(text, "CTRL-EVENT-STATE-CHANGE") ||
        g_str_has_prefix(text, "CTRL-EVENT-NETWORK-NOT-FOUND")
    )
    {
        wintc_net_device_queue_update(device, TRUE);
    }
}
