#ifndef __SHELL_DLGOPENW_H__
#define __SHELL_DLGOPENW_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_OPEN_WITH_DIALOG (wintc_sh_open_with_dialog_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShOpenWithDialog,
    wintc_sh_open_with_dialog,
    WINTC,
    SH_OPEN_WITH_DIALOG,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_sh_open_with_dialog_new(
    const gchar* file_path
);

#endif
