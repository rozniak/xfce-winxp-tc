#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc-comgtk.h>

#include "api.h"
#include "impl-wndmgmt-wnck.h"
#include "dll/wnck.h"

//
// FORWARD DECLARATIONS
//
static void wnck_window_unminimize_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_wnck_impl(void)
{
    if (!init_dll_wnck())
    {
        WINTC_LOG_USER_DEBUG("%s", "WNDMGMT: Can't use libwnck");
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
    wintc_wndmgmt_window_unminimize        = &wnck_window_unminimize_real;

    p_wnck_set_client_type(WNCK_CLIENT_TYPE_PAGER);

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void wnck_window_unminimize_real(
    WinTCWndMgmtWindow* window,
    guint64             timestamp
)
{
    p_wnck_window_unminimize(
        window,
        (guint32) timestamp
    );
}
