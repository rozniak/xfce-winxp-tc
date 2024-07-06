#ifndef __SETUPWND_H__
#define __SETUPWND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSetupWindowClass WinTCSetupWindowClass;
typedef struct _WinTCSetupWindow      WinTCSetupWindow;

#define WINTC_TYPE_SETUP_WINDOW            (wintc_setup_window_get_type())
#define WINTC_SETUP_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SETUP_WINDOW, WinTCSetupWindow))
#define WINTC_SETUP_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SETUP_WINDOW, WinTCSetupWindow))
#define IS_WINTC_SETUP_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SETUP_WINDOW))
#define IS_WINTC_SETUP_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SETUP_WINDOW))
#define WINTC_SETUP_WINDOW_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SETUP_WINDOW))

GType wintc_setup_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_window_new();

#endif
