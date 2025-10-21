#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_ICON_DBG_WINDOW (wintc_icon_dbg_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCIconDbgWindow,
    wintc_icon_dbg_window,
    WINTC,
    ICON_DBG_WINDOW,
    GtkApplicationWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_icon_dbg_window_new(
    WinTCIconDbgApplication* app
);

#endif
