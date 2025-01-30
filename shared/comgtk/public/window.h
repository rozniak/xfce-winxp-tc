/** @file */

#ifndef __COMGTK_WINDOW_H__
#define __COMGTK_WINDOW_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Requests that a window be given focus.
 *
 * @param window The window.
 */
void wintc_focus_window(
    GtkWindow* window
);

/**
 * Gets the top-level window that owns the widget, if there is no window, then
 * NULL will be returned.
 *
 * @param widget The widget.
 * @return The top-level window that owns the widget or NULL.
 */
GtkWindow* wintc_widget_get_toplevel_window(
    GtkWidget* widget
);

#endif
