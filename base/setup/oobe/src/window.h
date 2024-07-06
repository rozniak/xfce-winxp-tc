#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCOobeWindowClass WinTCOobeWindowClass;
typedef struct _WinTCOobeWindow      WinTCOobeWindow;

#define WINTC_TYPE_OOBE_WINDOW            (wintc_oobe_window_get_type())
#define WINTC_OOBE_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_OOBE_WINDOW, WinTCOobeWindow))
#define WINTC_OOBE_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_OOBE_WINDOW, WinTCOobeWindowClass))
#define IS_WINTC_OOBE_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_OOBE_WINDOW))
#define IS_WINTC_OOBE_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_OOBE_WINDOW))
#define WINTC_OOBE_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_OOBE_WINDOW, WinTCOobeWindowClass))

GType wintc_oobe_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_oobe_window_new(void);

#endif
