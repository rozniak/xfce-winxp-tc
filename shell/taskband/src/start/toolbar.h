#ifndef __TOOLBAR_START_H__
#define __TOOLBAR_START_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

#include "shared.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_START (wintc_toolbar_start_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarStart,
    wintc_toolbar_start,
    WINTC,
    TOOLBAR_START,
    WinTCShextUIController
)

//
// PUBLIC FUNCTIONS
//
void wintc_toolbar_start_toggle_menu(
    WinTCToolbarStart* toolbar_start
);

#endif

