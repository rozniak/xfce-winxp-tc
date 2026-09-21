#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSysAnalWindowClass WinTCSysAnalWindowClass;
typedef struct _WinTCSysAnalWindow      WinTCSysAnalWindow;

#define WINTC_TYPE_SYS_ANAL_WINDOW            (wintc_sys_anal_window_get_type())
#define WINTC_SYS_ANAL_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SYS_ANAL_WINDOW, WinTCSysAnalWindow))
#define WINTC_SYS_ANAL_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SYS_ANAL_WINDOW, WinTCSysAnalWindowClass))
#define IS_WINTC_SYS_ANAL_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SYS_ANAL_WINDOW))
#define IS_WINTC_SYS_ANAL_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SYS_ANAL_WINDOW))
#define WINTC_SYS_ANAL_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SYS_ANAL_WINDOW, WinTCSysAnalWindowClass))

GType wintc_sys_anal_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_sys_anal_window_new(
    WinTCSysAnalApplication* app
);

#endif
