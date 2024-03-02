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
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

/**
 * Callback for selecting a GtkMenuShell upon a notify-enter-event.
 */
void wintc_menu_shell_select_on_enter(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

#endif
