#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define TYPE_WINTC_NPWRDLG_APPLICATION (wintc_npwrdlg_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNewPwrDlgApplication,
    wintc_npwrdlg_application,
    WINTC,
    NPWRDLG_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCNewPwrDlgApplication* wintc_npwrdlg_application_new(void);

#endif
