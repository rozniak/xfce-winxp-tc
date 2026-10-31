#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

#include "device.h"
#include "util.h"

//
// STATIC DATA
//

// The XP icons live in the Luna theme under these names, the generic names
// after them are fallbacks for other icon themes
//
static const gchar* S_ICONS_WIRED_IDLE[] = {
    "network-idle",
    "network-wired",
    NULL
};
static const gchar* S_ICONS_WIRED_TRANSMIT[] = {
    "network-transmit",
    "network-idle",
    NULL
};
static const gchar* S_ICONS_WIRED_RECEIVE[] = {
    "network-receive",
    "network-idle",
    NULL
};
static const gchar* S_ICONS_WIRED_TRANSMIT_RECEIVE[] = {
    "network-transmit-receive",
    "network-idle",
    NULL
};
static const gchar* S_ICONS_WIRED_LIMITED[] = {
    "network-error",
    "network-wired-no-route",
    NULL
};
static const gchar* S_ICONS_WIRED_OFFLINE[] = {
    "network-wired-disconnected",
    "network-offline",
    NULL
};
static const gchar* S_ICONS_WIRED_ACQUIRING[WINTC_NET_ACQUIRING_FRAMES][4] = {
    {
        "network-wired-acquiring",
        "network-idle",
        NULL
    },
    {
        "network-wired-acquiring-2",
        "network-wired-acquiring",
        "network-idle",
        NULL
    },
    {
        "network-wired-acquiring-3",
        "network-wired-acquiring",
        "network-idle",
        NULL
    }
};

static const gchar* S_ICONS_WIRELESS_IDLE[] = {
    "network-wireless-connected",
    "network-wireless",
    NULL
};
static const gchar* S_ICONS_WIRELESS_TRANSMIT[] = {
    "network-wireless-transmit",
    "network-wireless-connected",
    "network-wireless",
    NULL
};
static const gchar* S_ICONS_WIRELESS_RECEIVE[] = {
    "network-wireless-receive",
    "network-wireless-connected",
    "network-wireless",
    NULL
};
static const gchar* S_ICONS_WIRELESS_TRANSMIT_RECEIVE[] = {
    "network-wireless-transmit-receive",
    "network-wireless-connected",
    "network-wireless",
    NULL
};
static const gchar* S_ICONS_WIRELESS_LIMITED[] = {
    "network-wireless-no-route",
    "network-error",
    NULL
};
static const gchar* S_ICONS_WIRELESS_DISCONNECTED[] = {
    "network-wireless-disconnected",
    "network-wireless-offline",
    "network-offline",
    NULL
};
static const gchar* S_ICONS_WIRELESS_DISABLED[] = {
    "network-wireless-offline",
    "network-wireless-disconnected",
    "network-offline",
    NULL
};
static const gchar*
    S_ICONS_WIRELESS_ACQUIRING[WINTC_NET_ACQUIRING_FRAMES][4] = {
    {
        "network-wireless-acquiring",
        "network-wireless",
        NULL
    },
    {
        "network-wireless-acquiring-2",
        "network-wireless-acquiring",
        "network-wireless",
        NULL
    },
    {
        "network-wireless-acquiring-3",
        "network-wireless-acquiring",
        "network-wireless",
        NULL
    }
};

static const gchar* S_ICONS_PREFERRED[] = {
    "network-wireless-preferred",
    "starred",
    "emblem-favorite",
    NULL
};
static const gchar* S_ICONS_SECURED[] = {
    "network-wireless-encrypted",
    "emblem-readonly",
    "changes-prevent",
    NULL
};

// XP has six levels of signal strength (no bars up to five bars)
//
static const gchar* S_ICONS_SIGNAL[WINTC_WLAN_SIGNAL_BARS_MAX + 1][3] = {
    { "network-wireless-signal-none",      NULL },
    { "network-wireless-signal-weak",      NULL },
    { "network-wireless-signal-ok",        NULL },
    { "network-wireless-signal-good",      NULL },
    {
        "network-wireless-signal-very-good",
        "network-wireless-signal-good",
        NULL
    },
    { "network-wireless-signal-excellent", NULL }
};

//
// GTK TYPE DEFINITIONS
//
G_DEFINE_QUARK(wintc-net-error-quark, wintc_net_error)

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

void wintc_net_ensure_styles(void)
{
    static gboolean s_styles_added = FALSE;

    // Add CSS once to screen - can't do this in class because GTK may not
    // have initialized first
    //
    if (s_styles_added)
    {
        return;
    }

    GtkCssProvider* css_net   = gtk_css_provider_new();
    GtkCssProvider* css_net_p = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_net,
        "/uk/oddmatics/wintc/taskband/net.css"
    );
    gtk_css_provider_load_from_resource(
        css_net_p,
        "/uk/oddmatics/wintc/taskband/net_p.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_net),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_net_p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(css_net);
    g_object_unref(css_net_p);

    s_styles_added = TRUE;
}

const gchar* const* wintc_net_get_activity_icon_names(
    WinTCNetActivity activity
)
{
    switch ((guint) activity)
    {
        case WINTC_NET_ACTIVITY_TRANSMIT:
            return S_ICONS_WIRED_TRANSMIT;

        case WINTC_NET_ACTIVITY_RECEIVE:
            return S_ICONS_WIRED_RECEIVE;

        case WINTC_NET_ACTIVITY_TRANSMIT | WINTC_NET_ACTIVITY_RECEIVE:
            return S_ICONS_WIRED_TRANSMIT_RECEIVE;

        default:
            return S_ICONS_WIRED_IDLE;
    }
}

const gchar* const* wintc_net_get_tray_icon_names(
    WinTCNetDeviceKind kind,
    WinTCNetState      state,
    WinTCNetActivity   activity,
    guint              frame
)
{
    frame %= WINTC_NET_ACQUIRING_FRAMES;

    if (kind == WINTC_NET_DEVICE_KIND_WIRED)
    {
        switch (state)
        {
            case WINTC_NET_STATE_CONNECTED:
                return wintc_net_get_activity_icon_names(activity);

            case WINTC_NET_STATE_LIMITED:
                return S_ICONS_WIRED_LIMITED;

            case WINTC_NET_STATE_CONNECTING:
            case WINTC_NET_STATE_ACQUIRING:
                return S_ICONS_WIRED_ACQUIRING[frame];

            default:
                return S_ICONS_WIRED_OFFLINE;
        }
    }

    switch (state)
    {
        case WINTC_NET_STATE_CONNECTED:
            switch ((guint) activity)
            {
                case WINTC_NET_ACTIVITY_TRANSMIT:
                    return S_ICONS_WIRELESS_TRANSMIT;

                case WINTC_NET_ACTIVITY_RECEIVE:
                    return S_ICONS_WIRELESS_RECEIVE;

                case WINTC_NET_ACTIVITY_TRANSMIT | WINTC_NET_ACTIVITY_RECEIVE:
                    return S_ICONS_WIRELESS_TRANSMIT_RECEIVE;

                default:
                    return S_ICONS_WIRELESS_IDLE;
            }

        case WINTC_NET_STATE_LIMITED:
            return S_ICONS_WIRELESS_LIMITED;

        case WINTC_NET_STATE_CONNECTING:
        case WINTC_NET_STATE_ACQUIRING:
            return S_ICONS_WIRELESS_ACQUIRING[frame];

        case WINTC_NET_STATE_DISABLED:
            return S_ICONS_WIRELESS_DISABLED;

        default:
            return S_ICONS_WIRELESS_DISCONNECTED;
    }
}

const gchar* const* wintc_wlan_get_preferred_icon_names(void)
{
    return S_ICONS_PREFERRED;
}

const gchar* const* wintc_wlan_get_secured_icon_names(void)
{
    return S_ICONS_SECURED;
}

const gchar* const* wintc_wlan_get_signal_icon_names(
    gint signal
)
{
    return S_ICONS_SIGNAL[wintc_wlan_signal_to_bars(signal)];
}

GdkPixbuf* wintc_net_load_icon(
    const gchar* const* icon_names,
    gint                size
)
{
    GError*      error = NULL;
    GtkIconInfo* icon_info;
    GdkPixbuf*   pixbuf;

    icon_info =
        gtk_icon_theme_choose_icon(
            gtk_icon_theme_get_default(),
            (const gchar**) icon_names,
            size,
            GTK_ICON_LOOKUP_FORCE_SIZE
        );

    if (!icon_info)
    {
        return NULL;
    }

    pixbuf = gtk_icon_info_load_icon(icon_info, &error);

    if (!pixbuf)
    {
        wintc_log_error_and_clear(&error);
    }

    g_object_unref(icon_info);

    return pixbuf;
}

void wintc_net_image_set_icon_names(
    GtkImage*           image,
    const gchar* const* icon_names,
    gint                size
)
{
    GIcon* icon = g_themed_icon_new_from_names((gchar**) icon_names, -1);

    gtk_image_set_from_gicon(image, icon, GTK_ICON_SIZE_BUTTON);
    gtk_image_set_pixel_size(image, size);

    g_object_unref(icon);
}

GtkWidget* wintc_wlan_create_signal_image(void)
{
    GtkWidget* image = gtk_image_new();

    gtk_widget_set_halign(image, GTK_ALIGN_END);
    gtk_widget_set_valign(image, GTK_ALIGN_END);

    wintc_widget_add_style_class(image, "wintc-wlan-signal");

    return image;
}

void wintc_wlan_signal_image_set_signal(
    GtkWidget* image,
    gint       signal
)
{
    wintc_net_image_set_icon_names(
        GTK_IMAGE(image),
        wintc_wlan_get_signal_icon_names(signal),
        32
    );

    gtk_widget_set_tooltip_text(
        image,
        wintc_wlan_signal_to_text(signal)
    );
}

gboolean wintc_net_tool_exists(
    const gchar* program
)
{
    gchar* path = g_find_program_in_path(program);

    if (!path)
    {
        return FALSE;
    }

    g_free(path);

    return TRUE;
}

void wintc_net_launch_tool(
    const gchar* cmdline,
    GtkWidget*   widget
)
{
    GError* error = NULL;

    if (!wintc_launch_command(cmdline, &error))
    {
        wintc_display_error_and_clear(
            &error,
            widget ? wintc_widget_get_toplevel_window(widget) : NULL
        );
    }
}

// The rest are for the backends, see client.h
//
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
