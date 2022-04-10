#ifndef __DEFPROCS_H__
#define __DEFPROCS_H__

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "shorthand.h"

//
// PUBLIC FUNCTIONS
//
void wintc_menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

void wintc_menu_shell_select_on_enter(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

#endif
