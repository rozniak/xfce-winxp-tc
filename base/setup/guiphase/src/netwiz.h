#ifndef __NETWIZ_H__
#define __NETWIZ_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/wizard97.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SETUP_NETWORK_WIZARD (wintc_setup_network_wizard_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSetupNetworkWizard,
    wintc_setup_network_wizard,
    WINTC,
    SETUP_NETWORK_WIZARD,
    WinTCWizard97Window
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_network_wizard_new(void);

#endif
