#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/icons.h"
#include "../public/strings.h"

#define WINTC_NET_ACQUIRING_FRAMES 3
#define WINTC_WLAN_SIGNAL_BARS_MAX 5

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
// PUBLIC FUNCTIONS
//
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
