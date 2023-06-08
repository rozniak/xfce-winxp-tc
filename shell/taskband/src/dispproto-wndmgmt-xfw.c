#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "dispproto.h"
#include "dispproto-wndmgmt-xfw.h"

//
// RESOLVED FUNCS
//
static WndMgmtWindow* (*p_xfw_screen_get_active_window) (
    WndMgmtScreen* screen
) = NULL;
static WndMgmtScreen* (*p_xfw_screen_get_default) (void) = NULL;

static GdkPixbuf* (*p_xfw_window_get_icon) (
    WndMgmtWindow* window,
    gint           size,
    gint           scale
) = NULL;
static gchar* (*p_xfw_window_get_name) (
    WndMgmtWindow* window
) = NULL;
static gboolean (*p_xfw_window_is_skip_tasklist) (
    WndMgmtWindow* window
) = NULL;
static gboolean (*p_xfw_window_set_minimized) (
    WndMgmtWindow* window,
    gboolean       is_maximized,
    GError**       error
) = NULL;

//
// FORWARD DECLARATIONS
//
static GdkPixbuf* xfw_window_get_mini_icon(
    WndMgmtWindow* window
);
static void xfw_window_minimize(
    WndMgmtWindow* window
);
static void xfw_window_unminimize(
    WndMgmtWindow* window
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
    wndmgmt_screen_get_active_window = p_xfw_screen_get_active_window;
    wndmgmt_screen_get_default       = p_xfw_screen_get_default;
    wndmgmt_window_get_mini_icon     = &xfw_window_get_mini_icon;
    wndmgmt_window_get_name          = p_xfw_window_get_name;
    wndmgmt_window_is_skip_tasklist  = p_xfw_window_is_skip_tasklist;
    wndmgmt_window_minimize          = &xfw_window_minimize;
    wndmgmt_window_unminimize        = &xfw_window_unminimize;

    return TRUE;
}

// PRIVATE FUNCTIONS
//
static GdkPixbuf* xfw_window_get_mini_icon(
    WndMgmtWindow* window
)
{
    return p_xfw_window_get_icon(window, 16, 1);
}

static void xfw_window_minimize(
    WndMgmtWindow* window
)
{
    p_xfw_window_set_minimized(window, TRUE, NULL);
}

static void xfw_window_unminimize(
    WndMgmtWindow* window
)
{
    p_xfw_window_set_minimized(window, FALSE, NULL);
}
