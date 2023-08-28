#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "api.h"
#include "impl-wndmgmt-wnck.h"
#include "dll/wnck.h"

//
// FORWARD DECLARATIONS
//
static void wnck_window_unminimize_now(
    WinTCWndMgmtWindow* window
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_wnck_impl(void)
{
    if (!init_dll_wnck())
    {
        g_warning("%s", "libwnck not available.");
        return FALSE;
    }

    // We're good, implement the API
    //
    wintc_wndmgmt_screen_get_active_window = p_wnck_screen_get_active_window;
    wintc_wndmgmt_screen_get_default       = p_wnck_screen_get_default;
    wintc_wndmgmt_window_get_mini_icon     = p_wnck_window_get_mini_icon;
    wintc_wndmgmt_window_get_name          = p_wnck_window_get_name;
    wintc_wndmgmt_window_is_skip_tasklist  = p_wnck_window_is_skip_tasklist;
    wintc_wndmgmt_window_minimize          = p_wnck_window_minimize;
    wintc_wndmgmt_window_unminimize        = &wnck_window_unminimize_now;

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void wnck_window_unminimize_now(
    WinTCWndMgmtWindow* window
)
{
    // FIXME: This throws a warning because we use 0 or GDK_CURRENT_TIME where
    //        it expects an X11 timestamp - it works and I can't be bothered to
    //        resolve this right now
    //
    p_wnck_window_unminimize(window, 0);
}
