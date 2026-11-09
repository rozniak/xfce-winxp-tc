#ifndef __NET_DEVICE_H__
#define __NET_DEVICE_H__

#include <gio/gio.h>
#include <glib.h>

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_NET_DEVICE_KIND_WIRED,
    WINTC_NET_DEVICE_KIND_WIRELESS
} WinTCNetDeviceKind;

typedef enum
{
    WINTC_NET_STATE_UNAVAILABLE,
    WINTC_NET_STATE_DISABLED,
    WINTC_NET_STATE_DISCONNECTED,
    WINTC_NET_STATE_CONNECTING,
    WINTC_NET_STATE_ACQUIRING,
    WINTC_NET_STATE_CONNECTED,
    WINTC_NET_STATE_LIMITED,
    WINTC_NET_STATE_DISCONNECTING
} WinTCNetState;

typedef enum
{
    WINTC_NET_ACTIVITY_NONE     = 0,
    WINTC_NET_ACTIVITY_TRANSMIT = 1 << 0,
    WINTC_NET_ACTIVITY_RECEIVE  = 1 << 1
} WinTCNetActivity;

typedef enum
{
    WINTC_WLAN_SECURITY_NONE,
    WINTC_WLAN_SECURITY_OWE,
    WINTC_WLAN_SECURITY_WEP,
    WINTC_WLAN_SECURITY_WPA,
    WINTC_WLAN_SECURITY_WPA2,
    WINTC_WLAN_SECURITY_WPA3,
    WINTC_WLAN_SECURITY_ENTERPRISE
} WinTCWlanSecurity;

typedef enum
{
    WINTC_NET_ERROR_FAILED,
    WINTC_NET_ERROR_NOT_FOUND,
    WINTC_NET_ERROR_SECRETS_REQUIRED
} WinTCNetError;

//
// PUBLIC DEFINES
//
#define WINTC_NET_ERROR (wintc_net_error_quark())

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCWlanNetwork
{
    gchar*            ssid;
    GBytes*           ssid_raw;
    gchar*            ap_path;
    gint              signal;
    gint              rate;
    WinTCWlanSecurity security;
    gboolean          in_use;

    gchar*            profile_uuid;
    gboolean          profile_autoconnect;
} WinTCWlanNetwork;

typedef struct _WinTCNetIpInfo
{
    gchar*   hwaddr;

    gchar*   address;
    gint     prefix;
    gchar*   gateway;
    gchar**  dns;

    gboolean dhcp;
    gchar*   dhcp_server;
    gint64   lease_obtained;
    gint64   lease_expires;
} WinTCNetIpInfo;

typedef struct _WinTCNetTraffic
{
    guint64 tx_bytes;
    guint64 rx_bytes;
    guint64 tx_packets;
    guint64 rx_packets;
} WinTCNetTraffic;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NET_DEVICE (wintc_net_device_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNetDevice,
    wintc_net_device,
    WINTC,
    NET_DEVICE,
    GObject
)

//
// PUBLIC FUNCTIONS
//
GQuark wintc_net_error_quark(void);

WinTCNetDeviceKind wintc_net_device_get_kind(
    WinTCNetDevice* device
);
const gchar* wintc_net_device_get_interface(
    WinTCNetDevice* device
);
WinTCNetState wintc_net_device_get_state(
    WinTCNetDevice* device
);
gboolean wintc_net_device_is_connected(
    WinTCNetDevice* device
);
gint64 wintc_net_device_get_connected_since(
    WinTCNetDevice* device
);
guint32 wintc_net_device_get_speed(
    WinTCNetDevice* device
);
const gchar* wintc_net_device_get_connection_uuid(
    WinTCNetDevice* device
);
WinTCNetIpInfo* wintc_net_device_get_ip_info(
    WinTCNetDevice* device
);
WinTCNetActivity wintc_net_device_get_activity(
    WinTCNetDevice* device
);
void wintc_net_device_get_traffic(
    WinTCNetDevice*  device,
    WinTCNetTraffic* traffic
);

gboolean wintc_net_device_get_radio_enabled(
    WinTCNetDevice* device
);
gboolean wintc_net_device_is_scanning(
    WinTCNetDevice* device
);
WinTCWlanNetwork* wintc_net_device_get_active_network(
    WinTCNetDevice* device
);
GList* wintc_net_device_get_networks(
    WinTCNetDevice* device
);
WinTCWlanNetwork* wintc_net_device_find_network(
    WinTCNetDevice* device,
    const gchar*    ssid
);
void wintc_net_device_request_scan(
    WinTCNetDevice* device
);

void wintc_net_device_connect_wlan_async(
    WinTCNetDevice*     device,
    const gchar*        ssid,
    const gchar*        key,
    GCancellable*       cancellable,
    GAsyncReadyCallback callback,
    gpointer            user_data
);
void wintc_net_device_disconnect_async(
    WinTCNetDevice*     device,
    GAsyncReadyCallback callback,
    gpointer            user_data
);
void wintc_net_device_repair_async(
    WinTCNetDevice*     device,
    GCancellable*       cancellable,
    GAsyncReadyCallback callback,
    gpointer            user_data
);
void wintc_net_device_set_enabled_async(
    WinTCNetDevice*     device,
    gboolean            enabled,
    GAsyncReadyCallback callback,
    gpointer            user_data
);
gboolean wintc_net_device_action_finish(
    WinTCNetDevice* device,
    GAsyncResult*   res,
    GError**        error
);

void wintc_net_ip_info_free(
    WinTCNetIpInfo* info
);

#endif
