#ifndef __STARTMENU_H__
#define __STARTMENU_H__

#include <gtk/gtk.h>

//
// STRUCTURE DEFINITIONS
//
typedef struct _StartMenu StartMenu;

//
// PUBLIC FUNCTIONS
//
void connect_start_menu_closed_signal(
    StartMenu* start_menu,
    GCallback  callback
);

void start_menu_close(
    StartMenu* start_menu
);

StartMenu* start_menu_new(
    GtkWidget* start_button
);

void start_menu_present(
    StartMenu* start_menu
);

#endif
