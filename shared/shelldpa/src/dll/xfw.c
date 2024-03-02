#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/api.h"
#include "xfw.h"

//
// STATIC DATA
//
static gboolean s_initialized = FALSE;

//
// RESOLVED FUNCS
//
WinTCWndMgmtWindow* (*p_xfw_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
) = NULL;
WinTCWndMgmtScreen* (*p_xfw_screen_get_default) (void) = NULL;

GdkPixbuf* (*p_xfw_window_get_icon) (
    WinTCWndMgmtWindow* window,
    gint                size,
    gint                scale
) = NULL;
gchar* (*p_xfw_window_get_name) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*p_xfw_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*p_xfw_window_set_minimized) (
    WinTCWndMgmtWindow* window,
    gboolean            is_minimized,
    GError**            error
) = NULL;

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_xfw()
{
    void* dl_xfw = NULL;

    if (s_initialized)
    {
        return TRUE;
    }

    dl_xfw = dlopen("libxfce4windowing-0.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_xfw == NULL)
    {
        WINTC_LOG_USER_DEBUG("%s", "libxfce4windowing not available");
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
        g_warning("%s", "libxfce4windowing loaded, but not all symbols");
        return FALSE;
    }

    s_initialized = TRUE;

    return TRUE;
}
