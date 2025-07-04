/** @file */

#ifndef __MSGINA_AUTHWND_H__
#define __MSGINA_AUTHWND_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "logon.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_GINA_AUTH_WINDOW (wintc_gina_auth_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCGinaAuthWindow,
    wintc_gina_auth_window,
    WINTC,
    GINA_AUTH_WINDOW,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCGinaAuthWindow.
 *
 * @param logon_session The logon session.
 * @return The new WinTCGinaAuthWindow instance cast to GtkWidget.
 */
GtkWidget* wintc_gina_auth_window_new(
    WinTCGinaLogonSession* logon_session
);

#endif

