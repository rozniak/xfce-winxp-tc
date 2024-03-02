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

#endif
