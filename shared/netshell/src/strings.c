#include <glib.h>
#include <glib/gi18n.h>

#include "../public/strings.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_net_get_connection_name(
    WinTCNetDeviceKind kind,
    guint              index
)
{
    // XP numbers additional connections of the same kind, starting from 2
    //
    if (kind == WINTC_NET_DEVICE_KIND_WIRELESS)
    {
        if (index == 0)
        {
            return g_strdup(_("Wireless Network Connection"));
        }

        return g_strdup_printf(
            _("Wireless Network Connection %u"),
            index + 1
        );
    }

    if (index == 0)
    {
        return g_strdup(_("Local Area Connection"));
    }

    return g_strdup_printf(_("Local Area Connection %u"), index + 1);
}

gchar* wintc_net_prefix_to_netmask(
    gint prefix
)
{
    if (prefix <= 0 || prefix > 32)
    {
        return g_strdup("0.0.0.0");
    }

    guint32 mask = 0xffffffffu << (32 - prefix);

    return g_strdup_printf(
        "%u.%u.%u.%u",
        (mask >> 24) & 0xff,
        (mask >> 16) & 0xff,
        (mask >> 8)  & 0xff,
        mask         & 0xff
    );
}

gchar* wintc_net_speed_to_text(
    guint32 kbps
)
{
    // XP shows one decimal place, eg. 100.0 Mbps
    //
    if (kbps >= 1000000)
    {
        guint32 tenths = kbps / 100000;

        return g_strdup_printf(_("%u.%u Gbps"), tenths / 10, tenths % 10);
    }

    if (kbps >= 1000)
    {
        guint32 tenths = kbps / 100;

        return g_strdup_printf(_("%u.%u Mbps"), tenths / 10, tenths % 10);
    }

    return g_strdup_printf(_("%u.0 Kbps"), kbps);
}

const gchar* wintc_net_state_to_text(
    WinTCNetDeviceKind kind,
    WinTCNetState      state
)
{
    switch (state)
    {
        case WINTC_NET_STATE_UNAVAILABLE:
            return kind == WINTC_NET_DEVICE_KIND_WIRED ?
                _("Network cable unplugged") :
                _("Wireless connection unavailable");

        case WINTC_NET_STATE_DISABLED:      return _("Disabled");
        case WINTC_NET_STATE_DISCONNECTED:  return _("Not connected");
        case WINTC_NET_STATE_CONNECTING:    return _("Connecting");
        case WINTC_NET_STATE_ACQUIRING:
            return _("Acquiring network address");
        case WINTC_NET_STATE_CONNECTED:     return _("Connected");
        case WINTC_NET_STATE_LIMITED:
            return _("Limited or no connectivity");
        case WINTC_NET_STATE_DISCONNECTING: return _("Disconnecting");
    }

    return "";
}

gint wintc_wlan_signal_to_bars(
    gint signal
)
{
    if (signal >= 80)
    {
        return 5;
    }
    else if (signal >= 60)
    {
        return 4;
    }
    else if (signal >= 40)
    {
        return 3;
    }
    else if (signal >= 20)
    {
        return 2;
    }
    else if (signal > 0)
    {
        return 1;
    }

    return 0;
}

const gchar* wintc_wlan_signal_to_text(
    gint signal
)
{
    switch (wintc_wlan_signal_to_bars(signal))
    {
        case 5:  return _("Excellent");
        case 4:  return _("Very Good");
        case 3:  return _("Good");
        case 2:  return _("Low");
        case 1:  return _("Very Low");
        default: return _("No Signal");
    }
}

gchar* wintc_wlan_security_to_text(
    WinTCWlanSecurity security
)
{
    const gchar* suffix = NULL;

    switch (security)
    {
        case WINTC_WLAN_SECURITY_NONE:
            return g_strdup(_("Unsecured wireless network"));

        case WINTC_WLAN_SECURITY_WEP:
            return g_strdup(_("Security-enabled wireless network"));

        case WINTC_WLAN_SECURITY_OWE:        suffix = "OWE";    break;
        case WINTC_WLAN_SECURITY_WPA:        suffix = "WPA";    break;
        case WINTC_WLAN_SECURITY_WPA2:       suffix = "WPA2";   break;
        case WINTC_WLAN_SECURITY_WPA3:       suffix = "WPA3";   break;
        case WINTC_WLAN_SECURITY_ENTERPRISE: suffix = "802.1X"; break;
    }

    if (!suffix)
    {
        return g_strdup(_("Security-enabled wireless network"));
    }

    return g_strdup_printf(
        _("Security-enabled wireless network (%s)"),
        suffix
    );
}
