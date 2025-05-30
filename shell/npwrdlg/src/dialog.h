#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define TYPE_WINTC_NPWRDLG_DIALOG (wintc_npwrdlg_dialog_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNewPwrDlgDialog,
    wintc_npwrdlg_dialog,
    WINTC,
    NPWRDLG_DIALOG,
    GtkApplicationWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_npwrdlg_dialog_new_for_power_options(
    WinTCNewPwrDlgApplication* app,
    WinTCGinaSmXfce*           sm_xfce
);

GtkWidget* wintc_npwrdlg_dialog_new_for_user_options(
    WinTCNewPwrDlgApplication* app,
    WinTCGinaSmXfce*           sm_xfce
);

#endif
