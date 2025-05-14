/** @file */

#ifndef __COMGTK_DEFPROCS_H__
#define __COMGTK_DEFPROCS_H__

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "shorthand.h"

//
// PUBLIC FUNCTIONS
//

/**
 * Callback for deselecting a GtkMenuShell upon a notify-leave-event.
 */
void wintc_menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
);

/**
 * Callback for selecting a GtkMenuShell upon a notify-enter-event.
 */
void wintc_menu_shell_select_on_enter(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
);

/**
 * Callback for closing the toplevel window upon a GtkButton clicked event.
 */
void wintc_button_close_window_on_clicked(
    GtkButton* self,
    gpointer   user_data
);

#endif
