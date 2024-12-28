#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_RESVWR_WINDOW (wintc_resvwr_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCResvwrWindow,
    wintc_resvwr_window,
    WINTC,
    RESVWR_WINDOW,
    GtkApplicationWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_resvwr_window_new(
    WinTCResvwrApplication* app
);

#endif
