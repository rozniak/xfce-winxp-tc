#ifndef __DISPPROTO_H__
#define __DISPPROTO_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC DEFINES
//
#define WndMgmtScreen void
#define WndMgmtWindow void

//
// PUBLIC ENUMS
//
typedef enum
{
    DISPPROTO_X11,
    DISPPROTO_WAYLAND
} TaskbandDisplayProtocol;

//
// PUBLIC FUNCTIONS
//
TaskbandDisplayProtocol get_display_protocol_in_use(void);
gboolean init_display_protocol_apis(void);

extern void (*anchor_taskband_to_bottom) (
    GtkWindow* taskband
);

extern WndMgmtWindow* (*wndmgmt_screen_get_active_window) (
    WndMgmtScreen* screen
);
extern WndMgmtScreen* (*wndmgmt_screen_get_default) (void);

extern GdkPixbuf* (*wndmgmt_window_get_mini_icon) (
    WndMgmtWindow* window
);
extern gchar* (*wndmgmt_window_get_name) (
    WndMgmtWindow* window
);
extern gboolean (*wndmgmt_window_is_skip_tasklist) (
    WndMgmtWindow* window
);
extern void (*wndmgmt_window_minimize) (
    WndMgmtWindow* window
);
extern void (*wndmgmt_window_unminimize) (
    WndMgmtWindow* window
);

#endif
