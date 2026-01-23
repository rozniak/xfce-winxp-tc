#ifndef __PERWIZ_H__
#define __PERWIZ_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/wizard97.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SETUP_PERSONALIZE_WIZARD (wintc_setup_personalize_wizard_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSetupPersonalizeWizard,
    wintc_setup_personalize_wizard,
    WINTC,
    SETUP_PERSONALIZE_WIZARD,
    WinTCWizard97Window
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_personalize_wizard_new(void);

#endif
