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
static void xfw_window_close(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);
static GdkPixbuf* xfw_window_get_mini_icon(
    WinTCWndMgmtWindow* window
);
static void xfw_window_maximize(
    WinTCWndMgmtWindow* window
);
static void xfw_window_minimize(
    WinTCWndMgmtWindow* window
);
static void xfw_window_unmaximize(
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
    wintc_wndmgmt_window_close             = &xfw_window_close;
    wintc_wndmgmt_window_get_mini_icon     = &xfw_window_get_mini_icon;
    wintc_wndmgmt_window_get_name          = p_xfw_window_get_name;
    wintc_wndmgmt_window_is_minimized      = p_xfw_window_is_minimized;
    wintc_wndmgmt_window_is_maximized      = p_xfw_window_is_maximized;
    wintc_wndmgmt_window_is_skip_tasklist  = p_xfw_window_is_skip_tasklist;
    wintc_wndmgmt_window_maximize          = &xfw_window_maximize;
    wintc_wndmgmt_window_minimize          = &xfw_window_minimize;
    wintc_wndmgmt_window_unmaximize        = &xfw_window_unmaximize;
    wintc_wndmgmt_window_unminimize        = &xfw_window_unminimize;

    return TRUE;
}

// PRIVATE FUNCTIONS
//
static void xfw_shutdown(void)
{
    // Nothing to do! xfce4windowing doesn't expose any shutdown method
}

static void xfw_window_close(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
)
{
    GError* error = NULL;

    p_xfw_window_close(window, timestamp, &error);

    wintc_log_error_and_clear(&error);
}

static GdkPixbuf* xfw_window_get_mini_icon(
    WinTCWndMgmtWindow* window
)
{
    GdkPixbuf* icon = p_xfw_window_get_icon(window, 16, 1);

    g_object_ref(icon);

    return icon;
}

static void xfw_window_maximize(
    WinTCWndMgmtWindow* window
)
{
    p_xfw_window_set_maximized(window, TRUE, NULL);
}

static void xfw_window_minimize(
    WinTCWndMgmtWindow* window
)
{
    p_xfw_window_set_minimized(window, TRUE, NULL);
}

static void xfw_window_unmaximize(
    WinTCWndMgmtWindow* window
)
{
    p_xfw_window_set_maximized(window, FALSE, NULL);
}

static void xfw_window_unminimize(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
)
{
    GError* error = NULL;

    p_xfw_window_activate(window, NULL, timestamp, &error);

    wintc_log_error_and_clear(&error);
}
