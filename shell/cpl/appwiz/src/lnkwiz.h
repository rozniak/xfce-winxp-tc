#ifndef __LNKWIZ_H__
#define __LNKWIZ_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/wizard97.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CPL_APPWIZ_NEW_LINK_WIZARD (wintc_cpl_appwiz_new_link_wizard_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCCplAppwizNewLinkWizard,
    wintc_cpl_appwiz_new_link_wizard,
    WINTC,
    CPL_APPWIZ_NEW_LINK_WIZARD,
    WinTCWizard97Window
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_appwiz_new_link_wizard_new(
    const gchar* path
);

#endif
