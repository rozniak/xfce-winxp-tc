#include <dlfcn.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/api.h"
#include "wnck.h"

//
// STATIC DATA
//
static gboolean s_initialized = FALSE;

//
// RESOLVED FUNCS
//
void (*p_wnck_set_client_type) (
    WnckClientType ewmh_sourceindication_client_type
) = NULL;

WinTCWndMgmtWindow* (*p_wnck_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
) = NULL;
WinTCWndMgmtScreen* (*p_wnck_screen_get_default) (void) = NULL;

void (*p_wnck_shutdown) (void) = NULL;

const gchar* (*p_wnck_window_get_class_instance_name) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*p_wnck_window_get_icon_is_fallback) (
    WinTCWndMgmtWindow* window
) = NULL;
GdkPixbuf* (*p_wnck_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
) = NULL;
gchar* (*p_wnck_window_get_name) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*p_wnck_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*p_wnck_window_is_minimized) (
    WinTCWndMgmtWindow* window
) = NULL;
gboolean (*p_wnck_window_is_maximized) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*p_wnck_window_minimize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*p_wnck_window_maximize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*p_wnck_window_unmaximize) (
    WinTCWndMgmtWindow* window
) = NULL;
void (*p_wnck_window_unminimize) (
    WinTCWndMgmtWindow* window,
    guint32             timestamp
) = NULL;
void (*p_wnck_window_close) (
    WinTCWndMgmtWindow* window,
    guint32             timestamp
) = NULL;

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_wnck()
{
    const gchar* dl_possible_names[] = {
        "libwnck-3.so",
        "libwnck-3.so.0",
        NULL
    };

    void* dl_wnck = NULL;

    if (s_initialized)
    {
        return TRUE;
    }

    for (int i = 0; dl_possible_names[i] != NULL; i++)
    {
        dl_wnck = dlopen(dl_possible_names[i], RTLD_LAZY | RTLD_LOCAL);

        if (dl_wnck != NULL)
        {
            break;
        }
    }

    if (dl_wnck == NULL)
    {
        WINTC_LOG_USER_DEBUG("%s", "libwnck not available.");
        return FALSE;
    }

    // Attempt to load the necessary functions
    //
    p_wnck_set_client_type =
        dlsym(dl_wnck, "wnck_set_client_type");

    p_wnck_screen_get_active_window =
        dlsym(dl_wnck, "wnck_screen_get_active_window");

    p_wnck_screen_get_default =
        dlsym(dl_wnck, "wnck_screen_get_default");

    p_wnck_shutdown =
        dlsym(dl_wnck, "wnck_shutdown");

    p_wnck_window_close =
        dlsym(dl_wnck, "wnck_window_close");   

    p_wnck_window_get_class_instance_name =
        dlsym(dl_wnck, "wnck_window_get_class_instance_name");

    p_wnck_window_get_icon_is_fallback =
        dlsym(dl_wnck, "wnck_window_get_icon_is_fallback");

    p_wnck_window_get_mini_icon =
        dlsym(dl_wnck, "wnck_window_get_mini_icon");

    p_wnck_window_get_name =
        dlsym(dl_wnck, "wnck_window_get_name");

    p_wnck_window_is_minimized =
        dlsym(dl_wnck, "wnck_window_is_minimized");
    
    p_wnck_window_is_maximized =
        dlsym(dl_wnck, "wnck_window_is_maximized");

    p_wnck_window_is_skip_tasklist =
        dlsym(dl_wnck, "wnck_window_is_skip_tasklist");

    p_wnck_window_maximize =
        dlsym(dl_wnck, "wnck_window_maximize");

    p_wnck_window_minimize =
        dlsym(dl_wnck, "wnck_window_minimize");

    p_wnck_window_unmaximize =
        dlsym(dl_wnck, "wnck_window_unmaximize");

    p_wnck_window_unminimize =
        dlsym(dl_wnck, "wnck_window_unminimize");
 
    // Check all symbols loaded
    //
    if (
        p_wnck_set_client_type                == NULL ||
        p_wnck_screen_get_active_window       == NULL ||
        p_wnck_screen_get_default             == NULL ||
        p_wnck_shutdown                       == NULL ||
        p_wnck_window_close                   == NULL ||
        p_wnck_window_get_class_instance_name == NULL ||
        p_wnck_window_get_icon_is_fallback    == NULL ||
        p_wnck_window_get_mini_icon           == NULL ||
        p_wnck_window_get_name                == NULL ||
        p_wnck_window_is_maximized            == NULL ||
        p_wnck_window_is_minimized            == NULL ||
        p_wnck_window_is_skip_tasklist        == NULL ||
        p_wnck_window_maximize                == NULL ||
        p_wnck_window_unmaximize              == NULL ||
        p_wnck_window_unminimize              == NULL
    )
    {
        g_warning("%s", "libwnck loaded, but not all symbols.");
        return FALSE;
    }

    s_initialized = TRUE;

    return TRUE;
}
