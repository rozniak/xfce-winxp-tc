#include <glib.h>

#include "../public/device.h"

//
// PUBLIC FUNCTIONS
//
void wintc_wlan_network_free(
    WinTCWlanNetwork* network
)
{
    g_free(network->ssid);
    g_free(network->ap_path);
    g_free(network->profile_uuid);
    g_clear_pointer(&(network->ssid_raw), g_bytes_unref);
    g_free(network);
}

gint wintc_wlan_network_compare(
    gconstpointer a,
    gconstpointer b
)
{
    const WinTCWlanNetwork* network_a = a;
    const WinTCWlanNetwork* network_b = b;

    // Connected network first, then the preferred ones, then by signal
    //
    if (network_a->in_use != network_b->in_use)
    {
        return network_a->in_use ? -1 : 1;
    }

    if (!(network_a->profile_uuid) != !(network_b->profile_uuid))
    {
        return network_a->profile_uuid ? -1 : 1;
    }

    if (network_a->signal != network_b->signal)
    {
        return network_b->signal - network_a->signal;
    }

    return g_utf8_collate(network_a->ssid, network_b->ssid);
}

void wintc_net_ip_info_free(
    WinTCNetIpInfo* info
)
{
    if (!info)
    {
        return;
    }

    g_free(info->hwaddr);
    g_free(info->address);
    g_free(info->gateway);
    g_strfreev(info->dns);
    g_free(info->dhcp_server);
    g_free(info);
}
