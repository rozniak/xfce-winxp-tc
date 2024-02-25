#ifndef __PERSONAL_START_H__
#define __PERSONAL_START_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "shared.h"

//
// INTERNAL FUNCTIONS
//
void close_personal_menu(
    WinTCToolbarStart* toolbar_start
);
void create_personal_menu(
    WinTCToolbarStart* toolbar_start
);
void destroy_personal_menu(
    WinTCToolbarStart* toolbar_start
);
void open_personal_menu(
    WinTCToolbarStart* toolbar_start
);

#endif
