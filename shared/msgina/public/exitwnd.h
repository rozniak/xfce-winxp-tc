#ifndef __GINA_EXITWND_H__
#define __GINA_EXITWND_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "xfsm.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_GINA_EXIT_WINDOW (wintc_gina_exit_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCGinaExitWindow,
    wintc_gina_exit_window,
    WINTC,
    GINA_EXIT_WINDOW,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_exit_window_new_for_power_options(
    WinTCGinaSmXfce* sm_xfce
);
GtkWidget* wintc_gina_exit_window_new_for_user_options(
    WinTCGinaSmXfce* sm_xfce
);

#endif
