#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_PKG_EXEC_APPLICATION (wintc_pkg_exec_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCPkgExecApplication,
    wintc_pkg_exec_application,
    WINTC,
    PKG_EXEC_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCPkgExecApplication* wintc_pkg_exec_application_new(void);

#endif
