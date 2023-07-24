#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "api.h"
#include "impl-wndmgmt-xfw.h"

//
// RESOLVED FUNCS
//
static WinTCWndMgmtWindow* (*p_xfw_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
) = NULL;
static WinTCWndMgmtScreen* (*p_xfw_screen_get_default) (void) = NULL;

static GdkPixbuf* (*p_xfw_window_get_icon) (
    WinTCWndMgmtWindow* window,
    gint                size,
    gint                scale
) = NULL;
static gchar* (*p_xfw_window_get_name) (
    WinTCWndMgmtWindow* window
) = NULL;
static gboolean (*p_xfw_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
) = NULL;
static gboolean (*p_xfw_window_set_minimized) (
    WinTCWndMgmtWindow* window,
    gboolean            is_maximized,
    GError**            error
) = NULL;

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
    void* dl_xfw = dlopen("libxfce4windowing-0.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_xfw == NULL)
    {
        g_message("%s", "libxfce4windowing not available.");
        return FALSE;
    }

    // Attempt to load the necessary functions
    //
    p_xfw_screen_get_active_window =
        dlsym(dl_xfw, "xfw_screen_get_active_window");

    p_xfw_screen_get_default =
        dlsym(dl_xfw, "xfw_screen_get_default");

    p_xfw_window_get_icon =
        dlsym(dl_xfw, "xfw_window_get_icon");

    p_xfw_window_get_name =
        dlsym(dl_xfw, "xfw_window_get_name");

    p_xfw_window_is_skip_tasklist =
        dlsym(dl_xfw, "xfw_window_is_skip_tasklist");

    p_xfw_window_set_minimized =
        dlsym(dl_xfw, "xfw_window_set_minimized");

    // Check all symbols loaded
    //
    if (
        p_xfw_screen_get_active_window == NULL ||
        p_xfw_screen_get_default       == NULL ||
        p_xfw_window_get_icon          == NULL ||
        p_xfw_window_get_name          == NULL ||
        p_xfw_window_is_skip_tasklist  == NULL ||
        p_xfw_window_set_minimized     == NULL
    )
    {
        g_warning("%s", "libxfce4windowing loaded, but not all symbols.");
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
