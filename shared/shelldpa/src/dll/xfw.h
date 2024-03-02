#ifndef __DLL_XFW_H__
#define __DLL_XFW_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "../../public/api.h"

//
// RESOLVED FUNCS
//
extern WinTCWndMgmtWindow* (*p_xfw_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
);
extern WinTCWndMgmtScreen* (*p_xfw_screen_get_default) (void);

extern GdkPixbuf* (*p_xfw_window_get_icon) (
    WinTCWndMgmtWindow* window,
    gint                size,
    gint                scale
);
extern gchar* (*p_xfw_window_get_name) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*p_xfw_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
);
extern void (*p_xfw_window_set_minimized) (
    WinTCWndMgmtWindow* window,
    gboolean            is_minimized,
    GError**            error
);

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_xfw(void);

#endif
