#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/wizard97.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WIZARD_WINDOW (wintc_wizard_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWizardWindow,
    wintc_wizard_window,
    WINTC,
    WIZARD_WINDOW,
    WinTCWizard97Window
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wizard_window_new(
    WinTCWizardApplication* app
);

#endif
