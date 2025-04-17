#ifndef __START_PROGMENU_H__
#define __START_PROGMENU_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_START_PROGMENU (wintc_toolbar_start_progmenu_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarStartProgmenu,
    wintc_toolbar_start_progmenu,
    WINTC,
    TOOLBAR_START_PROGMENU,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCToolbarStartProgmenu* wintc_toolbar_start_progmenu_new(void);

GtkWidget* wintc_toolbar_start_progmenu_new_gtk_menu(
    WinTCToolbarStartProgmenu* progmenu,
    WinTCCtlMenuBinding**      menu_binding
);

#endif
