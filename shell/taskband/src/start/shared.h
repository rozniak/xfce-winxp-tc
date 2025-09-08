#ifndef __SHARED_START_H__
#define __SHARED_START_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/shellext.h>

#include "progmenu.h"

//
// INTERNAL STRUCTS
//
typedef struct _PersonalStartMenuData
{
    // UI
    //
    GtkWidget* popup_menu;

    GtkWidget* button_logoff;
    GtkWidget* button_shutdown;
    GtkWidget* eventbox_userpic;
    GtkWidget* menubar_places;
    GtkWidget* menubar_programs;
    GtkWidget* menuitem_all_programs;
    GtkWidget* separator_all_programs;

    WinTCCtlMenuBinding* all_programs_binding;

    // UI state
    //
    gboolean sync_menu_refresh;

    // Custom style contexts
    //
    GtkStyleProvider* style_userpic;

    // Signal tuples
    //
    GArray* tuples_places;
    GArray* tuples_programs;
} PersonalStartMenuData;

typedef struct _WinTCToolbarStart
{
    WinTCShextUIController __parent__;

    // UI
    //
    GtkWidget* start_button;

    // Personal data struct
    //
    PersonalStartMenuData personal;

    WinTCToolbarStartProgmenu* progmenu;

    // UI state
    //
    gboolean sync_button;
    gboolean sync_menu_should_close;

    gint64 time_menu_closed;
} WinTCToolbarStart;

#endif
