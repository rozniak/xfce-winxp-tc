#ifndef __INSTWIZ_H__
#define __INSTWIZ_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/wizard97.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_PKG_EXEC_INSTALL_WIZARD (wintc_pkg_exec_install_wizard_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCPkgExecInstallWizard,
    wintc_pkg_exec_install_wizard,
    WINTC,
    PKG_EXEC_INSTALL_WIZARD,
    WinTCWizard97Window
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_pkg_exec_install_wizard_new(
    const gchar* path
);

#endif
