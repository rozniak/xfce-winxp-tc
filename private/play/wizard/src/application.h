#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WIZARD_APPLICATION (wintc_wizard_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWizardApplication,
    wintc_wizard_application,
    WINTC,
    WIZARD_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCWizardApplication* wintc_wizard_application_new(void);

#endif
