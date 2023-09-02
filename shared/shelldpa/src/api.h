#ifndef __API_H__
#define __API_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC DEFINES
//
#define WinTCWndMgmtScreen void
#define WinTCWndMgmtWindow void

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_DISPPROTO_X11,
    WINTC_DISPPROTO_WAYLAND
} WinTCDisplayProtocol;

//
// PUBLIC FUNCTIONS
//
WinTCDisplayProtocol wintc_get_display_protocol_in_use(void);
gboolean wintc_init_display_protocol_apis(void);

extern void (*wintc_anchor_taskband_to_bottom) (
    GtkWindow* taskband
);

extern void (*wintc_become_desktop_window) (
    GtkWindow* window
);

extern WinTCWndMgmtWindow* (*wintc_wndmgmt_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
);
extern WinTCWndMgmtScreen* (*wintc_wndmgmt_screen_get_default) (void);

extern GdkPixbuf* (*wintc_wndmgmt_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
);
extern gchar* (*wintc_wndmgmt_window_get_name) (
    WinTCWndMgmtWindow* window
);
extern gboolean (*wintc_wndmgmt_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
);
extern void (*wintc_wndmgmt_window_minimize) (
    WinTCWndMgmtWindow* window
);
extern void (*wintc_wndmgmt_window_unminimize) (
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

#endif
