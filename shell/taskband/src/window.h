#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shelldpa.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TASKBAND_WINDOW (wintc_taskband_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCTaskbandWindow,
    wintc_taskband_window,
    WINTC,
    TASKBAND_WINDOW,
    WinTCDpaPanelWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskband_window_new(
    WinTCTaskbandApplication* app
);

void wintc_taskband_window_toggle_start_menu(
    WinTCTaskbandWindow* taskband
);

#endif

