#ifndef __START_PROGMENU_H__
#define __START_PROGMENU_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
gboolean wintc_toolbar_start_progmenu_init(
    GError** error
);

GtkWidget* wintc_toolbar_start_progmenu_new_gtk_menu(void);

#endif
