#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/api.h"
#include "impl-wndmgmt-xfw.h"
#include "dll/xfw.h"

//
// FORWARD DECLARATIONS
//
static void xfw_shutdown(void);
static GdkPixbuf* xfw_window_get_mini_icon(
    WinTCWndMgmtWindow* window
);
static void xfw_window_minimize(
    WinTCWndMgmtWindow* window
);
static void xfw_window_unminimize(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_xfw_impl(void)
{
    if (!init_dll_xfw())
    {
        WINTC_LOG_USER_DEBUG("%s", "WNDMGMT: Can't use libxfce4windowing");
        return FALSE;
    }

    // We're good, implement the API
    //
    wintc_wndmgmt_screen_get_active_window = p_xfw_screen_get_active_window;
    wintc_wndmgmt_screen_get_default       = p_xfw_screen_get_default;
    wintc_wndmgmt_shutdown                 = &xfw_shutdown;
    wintc_wndmgmt_window_get_mini_icon     = &xfw_window_get_mini_icon;
    wintc_wndmgmt_window_get_name          = p_xfw_window_get_name;
    wintc_wndmgmt_window_is_skip_tasklist  = p_xfw_window_is_skip_tasklist;
    wintc_wndmgmt_window_minimize          = &xfw_window_minimize;
    wintc_wndmgmt_window_unminimize        = &xfw_window_unminimize;

    return TRUE;
}

// PRIVATE FUNCTIONS
//
static void xfw_shutdown(void)
{
    // Nothing to do! xfce4windowing doesn't expose any shutdown method
}

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
    WinTCWndMgmtWindow* window,
    WINTC_UNUSED(guint64 timestamp)
)
{
    GError* error = NULL;

    p_xfw_window_set_minimized(window, FALSE, &error);

    wintc_log_error_and_clear(&error);
}
