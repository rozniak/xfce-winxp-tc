#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNewPwrDlgApplicationClass WinTCNewPwrDlgApplicationClass;
typedef struct _WinTCNewPwrDlgApplication      WinTCNewPwrDlgApplication;

#define TYPE_WINTC_NPWRDLG_APPLICATION            (wintc_npwrdlg_application_get_type())
#define WINTC_NPWRDLG_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_NPWRDLG_APPLICATION, WinTCNewPwrDlgApplication))
#define WINTC_NPWRDLG_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_NPWRDLG_APPLICATION, WinTCNewPwrDlgApplicationClass))
#define IS_WINTC_NPWRDLG_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_NPWRDLG_APPLICATION))
#define IS_WINTC_NPWRDLG_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_NPWRDLG_APPLICATION))
#define WINTC_NPWRDLG_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_NPWRDLG_APPLICATION, WinTCNewPwrDlgApplicationClass))

GType wintc_npwrdlg_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCNewPwrDlgApplication* wintc_npwrdlg_application_new(void);

#endif
