#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "settings.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_LOGONUI_WINDOW (wintc_logonui_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCLogonUIWindow,
    wintc_logonui_window,
    WINTC,
    LOGONUI_WINDOW,
    GtkWindow
);

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_logonui_window_new(
    WinTCLogonUISettings* settings
);

#endif
