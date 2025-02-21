#ifndef __SHARED_START_H__
#define __SHARED_START_H__

#include <garcon/garcon.h>

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

    // UI state
    //
    gboolean sync_menu_refresh;

    // Custom style contexts
    //
    GtkStyleProvider* style_userpic;

    // Garcon stuff
    //
    GarconMenu* garcon_all_entries;

    // Signal tuples
    //
    GArray* tuples_places;
    GArray* tuples_programs;
} PersonalStartMenuData;

typedef struct _WinTCToolbarStartClass
{
    WinTCTaskbandToolbarClass __parent__;
} WinTCToolbarStartClass;

typedef struct _WinTCToolbarStart
{
    WinTCTaskbandToolbar __parent__;

    // Personal data struct
    //
    PersonalStartMenuData personal;

    // UI state
    //
    gboolean sync_button;
    gboolean sync_menu_should_close;

    gint64 time_menu_closed;
} WinTCToolbarStart;

#endif
