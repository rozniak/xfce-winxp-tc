#ifndef __DLL_WNCK_H__
#define __DLL_WNCK_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>

#include "../../public/api.h"

//
// PUBLIC ENUMS
//
typedef enum
{
    WNCK_CLIENT_TYPE_APPLICATION = 1,
    WNCK_CLIENT_TYPE_PAGER       = 2
} WnckClientType;

//
// RESOLVED FUNCS
//
extern void (*p_wnck_set_client_type) (
    WnckClientType ewmh_sourceindication_client_type
);

extern WinTCWndMgmtWindow* (*p_wnck_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
);
extern WinTCWndMgmtScreen* (*p_wnck_screen_get_default) (void);

extern void (*p_wnck_shutdown) (void);

extern void (*p_wnck_window_close) (
    WinTCWndMgmtWindow* window,
    guint32             timestamp
);
extern const gchar* (*p_wnck_window_get_class_instance_name) (
    WinTCWndMgmtWindow* Window
);
extern gboolean (*p_wnck_window_get_icon_is_fallback) (
    WinTCWndMgmtWindow* window
);
extern GdkPixbuf* (*p_wnck_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
);
extern gchar* (*p_wnck_window_get_name) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*p_wnck_window_is_maximized) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*p_wnck_window_is_minimized) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*p_wnck_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
);
extern void (*p_wnck_window_maximize) (
    WinTCWndMgmtWindow* window
);
extern void (*p_wnck_window_minimize) (
    WinTCWndMgmtWindow* window
);
extern void (*p_wnck_window_unmaximize) (
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

