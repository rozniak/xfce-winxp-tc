#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNewPwrDlgDialogPrivate WinTCNewPwrDlgDialogPrivate;
typedef struct _WinTCNewPwrDlgDialogClass   WinTCNewPwrDlgDialogClass;
typedef struct _WinTCNewPwrDlgDialog        WinTCNewPwrDlgDialog;

#define TYPE_WINTC_NPWRDLG_DIALOG            (wintc_npwrdlg_dialog_get_type())
#define WINTC_NPWRDLG_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_NPWRDLG_DIALOG, WinTCNewPwrDlgDialog))
#define WINTC_NPWRDLG_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_NPWRDLG_DIALOG, WinTCNewPwrDlgDialogClass))
#define IS_WINTC_NPWRDLG_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_NPWRDLG_DIALOG))
#define IS_WINTC_NPWRDLG_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_NPWRDLG_DIALOG))
#define WINTC_NPWRDLG_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_NPWRDLG_DIALOG, WinTCNewPwrDlgDialogClass))

GType wintc_npwrdlg_dialog_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_npwrdlg_dialog_new(
    WinTCNewPwrDlgApplication* app
);

#endif
