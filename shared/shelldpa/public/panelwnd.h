#ifndef __SHELLDPA_PANELWND_H__
#define __SHELLDPA_PANELWND_H__

#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_DPA_PANEL_WINDOW (wintc_dpa_panel_window_get_type())

G_DECLARE_DERIVABLE_TYPE(
    WinTCDpaPanelWindow,
    wintc_dpa_panel_window,
    WINTC,
    DPA_PANEL_WINDOW,
    GtkApplicationWindow
)

struct _WinTCDpaPanelWindowClass
{
    GtkApplicationWindowClass __parent__;
};

//
// PUBLIC FUNCTIONS
//
GdkGravity wintc_dpa_panel_window_get_dock_side(
    WinTCDpaPanelWindow* wnd
);
void wintc_dpa_panel_window_set_dock_side(
    WinTCDpaPanelWindow* wnd,
    GdkGravity           dock_side
);

#endif
