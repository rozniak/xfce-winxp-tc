#ifndef __SETUPWND_H__
#define __SETUPWND_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shelldpa.h>

//
// PUBLIC ENUMS
//
enum
{
    WINTC_SETUP_STEP_COLLECTING_INFORMATION = 0,
    WINTC_SETUP_STEP_DYNAMIC_UPDATE,
    WINTC_SETUP_STEP_PREPARING_INSTALLATION,
    WINTC_SETUP_STEP_INSTALLING_WINDOWS,
    WINTC_SETUP_STEP_FINALIZING_INSTALLATION,
    N_WINTC_SETUP_STEPS
};

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SETUP_WINDOW (wintc_setup_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSetupWindow,
    wintc_setup_window,
    WINTC,
    SETUP_WINDOW,
    WinTCDpaDesktopWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_window_new();

void wintc_setup_window_disable_throbbers(
    WinTCSetupWindow* wnd
);
void wintc_setup_window_enable_throbbers(
    WinTCSetupWindow* wnd
);
void wintc_setup_window_set_completion_minutes_approx(
    WinTCSetupWindow* wnd,
    guint             minutes
);
void wintc_setup_window_set_current_step(
    WinTCSetupWindow* wnd,
    gint              step
);

#endif
