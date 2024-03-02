#ifndef __DIALOG_H__
#define __DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCRunDialogClass WinTCRunDialogClass;
typedef struct _WinTCRunDialog      WinTCRunDialog;

#define WINTC_TYPE_RUN_DIALOG            (wintc_run_dialog_get_type())
#define WINTC_RUN_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_RUN_DIALOG, WinTCRunDialog))
#define WINTC_RUN_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_RUN_DIALOG, WinTCRunDialogClass))
#define IS_WINTC_RUN_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_RUN_DIALOG))
#define IS_WINTC_RUN_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_RUN_DIALOG))
#define WINTC_RUN_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_RUN_DIALOG, WinTCRunDialogClass))

GType wintc_run_dialog_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_run_dialog_new(
    WinTCRunApplication* app
);

#endif
