#ifndef __OOBEWND_H__
#define __OOBEWND_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shelldpa.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_OOBE_WINDOW (wintc_oobe_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCOobeWindow,
    wintc_oobe_window,
    WINTC,
    OOBE_WINDOW,
    WinTCDpaDesktopWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_oobe_window_new(void);

#endif
