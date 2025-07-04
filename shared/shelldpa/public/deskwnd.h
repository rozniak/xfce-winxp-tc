#ifndef __SHELLDPA_DESKWND_H__
#define __SHELLDPA_DESKWND_H__

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_DPA_DESKTOP_WINDOW (wintc_dpa_desktop_window_get_type())

G_DECLARE_DERIVABLE_TYPE(
    WinTCDpaDesktopWindow,
    wintc_dpa_desktop_window,
    WINTC,
    DPA_DESKTOP_WINDOW,
    GtkApplicationWindow
)

typedef struct _WinTCDpaDesktopWindowClass
{
    GtkApplicationWindowClass __parent__;
} WinTCDpaDesktopWindowClass;

//
// PUBLIC FUNCTIONS
//
GdkMonitor* wintc_dpa_desktop_window_get_monitor(
    WinTCDpaDesktopWindow* wnd
);

#endif
