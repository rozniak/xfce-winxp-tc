#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "api.h"
#include "impl-wndmgmt-xfw.h"
#include "dll/xfw.h"

//
// FORWARD DECLARATIONS
//
static GdkPixbuf* xfw_window_get_mini_icon(
    WinTCWndMgmtWindow* window
);
static void xfw_window_minimize(
    WinTCWndMgmtWindow* window
);
static void xfw_window_unminimize(
    WinTCWndMgmtWindow* window
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_xfw_impl(void)
{
    if (!init_dll_xfw())
    {
        g_warning("%s", "libxfce4windowing not available.");
        return FALSE;
    }

    // We're good, implement the API
    //
    wintc_wndmgmt_screen_get_active_window = p_xfw_screen_get_active_window;
    wintc_wndmgmt_screen_get_default       = p_xfw_screen_get_default;
    wintc_wndmgmt_window_get_mini_icon     = &xfw_window_get_mini_icon;
    wintc_wndmgmt_window_get_name          = p_xfw_window_get_name;
    wintc_wndmgmt_window_is_skip_tasklist  = p_xfw_window_is_skip_tasklist;
    wintc_wndmgmt_window_minimize          = &xfw_window_minimize;
    wintc_wndmgmt_window_unminimize        = &xfw_window_unminimize;

    return TRUE;
}

// PRIVATE FUNCTIONS
//
static GdkPixbuf* xfw_window_get_mini_icon(
    WinTCWndMgmtWindow* window
)
{
    return p_xfw_window_get_icon(window, 16, 1);
}

static void xfw_window_minimize(
    WinTCWndMgmtWindow* window
)
{
    p_xfw_window_set_minimized(window, TRUE, NULL);
}

static void xfw_window_unminimize(
    WinTCWndMgmtWindow* window
)
{
    p_xfw_window_set_minimized(window, FALSE, NULL);
}
