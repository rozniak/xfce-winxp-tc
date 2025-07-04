#ifndef __WELCOME_UI_H__
#define __WELCOME_UI_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/msgina.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WELCOME_UI (wintc_welcome_ui_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWelcomeUI,
    wintc_welcome_ui,
    WINTC,
    WELCOME_UI,
    GtkContainer
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_welcome_ui_new(
    WinTCGinaLogonSession* logon_session
);

#endif
