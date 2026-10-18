#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_shwnd.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_LOCATION_CHANGED = 0,
    SIGNAL_MODE_CHANGED,
    N_SIGNALS
};

//
// STATIC DATA
//
static gint wintc_ishext_shell_window_signals[N_SIGNALS] = { 0 };

//
// GTK INTERFACE DEFINITIONS & CTORS
//
G_DEFINE_INTERFACE(
    WinTCIShextShellWindow,
    wintc_ishext_shell_window,
    G_TYPE_OBJECT
)

static void wintc_ishext_shell_window_default_init(
    WinTCIShextShellWindowInterface* iface
)
{
    GType iface_type = G_TYPE_FROM_INTERFACE(iface);

    wintc_ishext_shell_window_signals[SIGNAL_LOCATION_CHANGED] =
        g_signal_new(
            "location-changed",
            iface_type,
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_ishext_shell_window_signals[SIGNAL_MODE_CHANGED] =
        g_signal_new(
            "mode-changed",
            iface_type,
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

//
// INTERFACE METHODS
//
void wintc_ishext_shell_window_get_location(
    WinTCIShextShellWindow* shwnd,
    WinTCShextPathInfo*     path_info
)
{
    WinTCIShextShellWindowInterface* iface =
        WINTC_ISHEXT_SHELL_WINDOW_GET_IFACE(shwnd);

    iface->get_location(shwnd, path_info);
}

WinTCShextBrowserMode wintc_ishext_shell_window_get_mode(
    WinTCIShextShellWindow* shwnd
)
{
    WinTCIShextShellWindowInterface* iface =
        WINTC_ISHEXT_SHELL_WINDOW_GET_IFACE(shwnd);

    return iface->get_mode(shwnd);
}

gboolean wintc_ishext_shell_window_navigate(
    WinTCIShextShellWindow*   shwnd,
    const WinTCShextPathInfo* path_info,
    GError**                  error
)
{
    WinTCIShextShellWindowInterface* iface =
        WINTC_ISHEXT_SHELL_WINDOW_GET_IFACE(shwnd);

    return iface->navigate(shwnd, path_info, error);
}

gboolean wintc_ishext_shell_window_navigate_simple(
    WinTCIShextShellWindow* shwnd,
    const gchar*            path,
    GError**                error
)
{
    WinTCIShextShellWindowInterface* iface =
        WINTC_ISHEXT_SHELL_WINDOW_GET_IFACE(shwnd);

    return iface->navigate_simple(shwnd, path, error);
}
