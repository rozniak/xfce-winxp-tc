#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCTaskmgrWindowClass WinTCTaskmgrWindowClass;
typedef struct _WinTCTaskmgrWindow      WinTCTaskmgrWindow;

#define WINTC_TYPE_TASKMGR_WINDOW            (wintc_taskmgr_window_get_type())
#define WINTC_TASKMGR_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_TASKMGR_WINDOW, WinTCTaskmgrWindow))
#define WINTC_TASKMGR_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_TASKMGR_WINDOW, WinTCTaskmgrWindowClass))
#define IS_WINTC_TASKMGR_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_TASKMGR_WINDOW))
#define IS_WINTC_TASKMGR_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_TASKMGR_WINDOW))
#define WINTC_TASKMGR_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_TASKMGR_WINDOW, WinTCTaskmgrWindowClass))

GType wintc_taskmgr_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskmgr_window_new(
    WinTCTaskmgrApplication* app
);

#endif
