#ifndef __DLL_WNCK_H__
#define __DLL_WNCK_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "../api.h"

//
// RESOLVED FUNCS
//
extern WinTCWndMgmtWindow* (*p_wnck_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
);
extern WinTCWndMgmtScreen* (*p_wnck_screen_get_default) (void);

extern GdkPixbuf* (*p_wnck_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
);
extern gchar* (*p_wnck_window_get_name) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*p_wnck_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
);
extern void (*p_wnck_window_minimize) (
    WinTCWndMgmtWindow* window
);
extern void (*p_wnck_window_unminimize) (
    WinTCWndMgmtWindow* window,
    guint32             timestamp
);

//
// PUBLIC FUNCTIONS
//
gboolean init_dll_wnck(void);

#endif
