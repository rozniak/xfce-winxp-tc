#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>

#include "application.h"
#include "settings.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_DESKTOP_WINDOW (wintc_desktop_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCDesktopWindow,
    wintc_desktop_window,
    WINTC,
    DESKTOP_WINDOW,
    WinTCDpaDesktopWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_desktop_window_new(
    WinTCDesktopApplication* app,
    GdkMonitor*              monitor,
    WinTCDesktopSettings*    settings,
    WinTCShextHost*          shext_host
);

#endif
