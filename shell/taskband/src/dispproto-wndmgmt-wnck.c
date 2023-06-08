#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "dispproto.h"
#include "dispproto-wndmgmt-wnck.h"

//
// RESOLVED FUNCS
//
static WndMgmtWindow* (*p_wnck_screen_get_active_window) (
    WndMgmtScreen* screen
) = NULL;
static WndMgmtScreen* (*p_wnck_screen_get_default) (void) = NULL;

static GdkPixbuf* (*p_wnck_window_get_mini_icon) (
    WndMgmtWindow* window
) = NULL;
static gchar* (*p_wnck_window_get_name) (
    WndMgmtWindow* window
) = NULL;
static gboolean (*p_wnck_window_is_skip_tasklist) (
    WndMgmtWindow* window
) = NULL;
static void (*p_wnck_window_minimize) (
    WndMgmtWindow* window
) = NULL;
static void (*p_wnck_window_unminimize) (
    WndMgmtWindow* window,
    guint32 timestamp
) = NULL;

//
// FORWARD DECLARATIONS
//
static void wnck_window_unminimize_now(
    WndMgmtWindow* window
);

//
// PUBLIC FUNCTIONS
//
gboolean init_wndmgmt_wnck_impl(void)
{
    void* dl_wnck = dlopen("libwnck-3.so", RTLD_LAZY | RTLD_LOCAL);

    if (dl_wnck == NULL)
    {
        g_message("%s", "libwnck not available.");
        return FALSE;
    }

    // Attempt to load the necessary functions
    //
    p_wnck_screen_get_active_window =
        dlsym(dl_wnck, "wnck_screen_get_active_window");

    p_wnck_screen_get_default =
        dlsym(dl_wnck, "wnck_screen_get_default");

    p_wnck_window_get_mini_icon =
        dlsym(dl_wnck, "wnck_window_get_mini_icon");

    p_wnck_window_get_name =
        dlsym(dl_wnck, "wnck_window_get_name");

    p_wnck_window_is_skip_tasklist =
        dlsym(dl_wnck, "wnck_window_is_skip_tasklist");

    p_wnck_window_minimize =
        dlsym(dl_wnck, "wnck_window_minimize");

    p_wnck_window_unminimize =
        dlsym(dl_wnck, "wnck_window_unminimize");

    // Check all symbols loaded
    //
    if (
        p_wnck_screen_get_active_window == NULL ||
        p_wnck_screen_get_default       == NULL ||
        p_wnck_window_get_mini_icon     == NULL ||
        p_wnck_window_get_name          == NULL ||
        p_wnck_window_is_skip_tasklist  == NULL ||
        p_wnck_window_minimize          == NULL ||
        p_wnck_window_unminimize        == NULL
    )
    {
        g_warning("%s", "libwnck loaded, but not all symbols.");
        return FALSE;
    }

    // We're good, implement the API
    //
    wndmgmt_screen_get_active_window = p_wnck_screen_get_active_window;
    wndmgmt_screen_get_default       = p_wnck_screen_get_default;
    wndmgmt_window_get_mini_icon     = p_wnck_window_get_mini_icon;
    wndmgmt_window_get_name          = p_wnck_window_get_name;
    wndmgmt_window_is_skip_tasklist  = p_wnck_window_is_skip_tasklist;
    wndmgmt_window_minimize          = p_wnck_window_minimize;
    wndmgmt_window_unminimize        = &wnck_window_unminimize_now;

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void wnck_window_unminimize_now(
    WndMgmtWindow* window
)
{
    // FIXME: This throws a warning because we use 0 or GDK_CURRENT_TIME where
    //        it expects an X11 timestamp - it works and I can't be bothered to
    //        resolve this right now
    //
    p_wnck_window_unminimize(window, 0);
}
