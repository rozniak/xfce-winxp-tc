#ifndef __CLASSIC_UI_H__
#define __CLASSIC_UI_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/msgina.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CLASSIC_UI (wintc_classic_ui_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCClassicUI,
    wintc_classic_ui,
    WINTC,
    CLASSIC_UI,
    GtkWidget
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_classic_ui_new(
    WinTCGinaLogonSession* logon_session
);

#endif
