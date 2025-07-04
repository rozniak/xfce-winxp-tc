#ifndef __SETUPWND_H__
#define __SETUPWND_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shelldpa.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SETUP_WINDOW (wintc_setup_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSetupWindow,
    wintc_setup_window,
    WINTC,
    SEUTP_WINDOW,
    WinTCDpaDesktopWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_window_new();

#endif
