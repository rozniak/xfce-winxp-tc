/**
 * @file
 *
 * This specifies the main API for the display protocol abstraction library.
 *
 * This library is subject to change massively, as the current iteration is
 * much more akin to a 'polyfill'. It is a temporary solution between X11 and
 * Wayland, until further testing and development is done to ensure proper
 * support.
 *
 * The APIs provided here relate to things that involve talking over the
 * display protocol for their functionality. Things such as window management
 * and creating shell windows (eg. docked panels, the desktop).
 */

#ifndef __SHELLDPA_API_H__
#define __SHELLDPA_API_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC DEFINES
//

/**
 * Represents a screen/workspace for window management.
 */
#define WinTCWndMgmtScreen void

/**
 * Represents a single window for window management.
 */
#define WinTCWndMgmtWindow void

//
// PUBLIC ENUMS
//

/**
 * Specifies display protocols that are supported.
 */
typedef enum
{
    /** The X11 display protocol. */
    WINTC_DISPPROTO_X11,
    /** The Wayland display protocol. */
    WINTC_DISPPROTO_WAYLAND
} WinTCDisplayProtocol;

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a widget that can be used as a popup on the active display
 * protocol.
 *
 * @param owner              The widget to tie the popup to.
 * @param enable_composition True if composition is needed (for shadows etc.)
 * @return The appropriate widget cast to GtkWidget.
 */
GtkWidget* wintc_dpa_create_popup(
    GtkWidget* owner,
    gboolean   enable_composition
);

/**
 * Shows a popup.
 *
 * @param popup The popup.
 * @param owner The widget to tie the popup to.
 */
void wintc_dpa_show_popup(
    GtkWidget* popup,
    GtkWidget* owner
);

/**
 * Retrieves the active display protocol.
 *
 * @return The active display protocol.
 */
WinTCDisplayProtocol wintc_get_display_protocol_in_use(void);

/**
 * Initializes the library functionality.
 *
 * @return True if initialization was successful.
 */
gboolean wintc_init_display_protocol_apis(void);

/**
 * Anchors the window to the bottom of the primary display, assuming it is the
 * taskband.
 *
 * @param taskband The taskband window.
 *
 * @remarks This function will be retired with a better, general purpose API.
 */
extern void (*wintc_anchor_taskband_to_bottom) (
    GtkWindow* taskband
);

/**
 * Retrieves the active window on the specified screen.
 *
 * @param screen The screen.
 * @return The active window.
 */
extern WinTCWndMgmtWindow* (*wintc_wndmgmt_screen_get_active_window) (
    WinTCWndMgmtScreen* screen
);

/**
 * Retrieves the default screen.
 *
 * @return The default screen.
 */
extern WinTCWndMgmtScreen* (*wintc_wndmgmt_screen_get_default) (void);

/**
 * Shuts down window management functionality, releasing all resources.
 */
extern void (*wintc_wndmgmt_shutdown) (void);

/**
 * Closes the specified window.
 *
 * @param window    The window.
 * @param timestamp The timestamp, for focus stealing prevention.
 */
extern void (*wintc_wndmgmt_window_close) (
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

/**
 * Retrieves the icon for the specified window.
 *
 * @param window The window.
 * @return A pixmap of the icon.
 */
extern GdkPixbuf* (*wintc_wndmgmt_window_get_mini_icon) (
    WinTCWndMgmtWindow* window
);

/**
 * Retrieves the title of the specified window.
 *
 * @param window The window.
 * @return The window title.
 */
extern gchar* (*wintc_wndmgmt_window_get_name) (
    WinTCWndMgmtWindow* window
);

/**
 * Checks if the specified window is maximized.
 *
 * @param window The window.
 * @return True if the window is maximized.
 */
extern gboolean (*wintc_wndmgmt_window_is_maximized) (
    WinTCWndMgmtWindow* window
);

/**
 * Checks if the specified window is minimized.
 *
 * @param window The window.
 * @return True if the window is minimized.
 */
extern gboolean (*wintc_wndmgmt_window_is_minimized) (
    WinTCWndMgmtWindow* window
);

/**
 * Determines whether the specified window should receive a button in the
 * window switcher.
 *
 * @param window The window.
 * @return True if the window should have a button in the window switcher.
 */
extern gboolean (*wintc_wndmgmt_window_is_skip_tasklist) (
    WinTCWndMgmtWindow* window
);

/**
 * Maximizes the specified window.
 *
 * @param window The window.
 */
extern void (*wintc_wndmgmt_window_maximize) (
    WinTCWndMgmtWindow* window
);

/**
 * Minimizes the specified window.
 *
 * @param window The window.
 */
extern void (*wintc_wndmgmt_window_minimize) (
    WinTCWndMgmtWindow* window
);

/**
 * Unmaximizes the specified window.
 *
 * @param window The window.
 */
extern void (*wintc_wndmgmt_window_unmaximize) (
    WinTCWndMgmtWindow* window
);

/**
 * Unminimizes the specified window.
 *
 * @param window    The window.
 * @param timestamp The timestamp, for focus stealing prevention.
 */
extern void (*wintc_wndmgmt_window_unminimize) (
    WinTCWndMgmtWindow* window,
    guint64             timestamp
);

#endif
