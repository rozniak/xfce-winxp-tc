#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCHelloWindowClass WinTCHelloWindowClass;
typedef struct _WinTCHelloWindow      WinTCHelloWindow;

#define WINTC_TYPE_HELLO_WINDOW            (wintc_hello_window_get_type())
#define WINTC_HELLO_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_HELLO_WINDOW, WinTCHelloWindow))
#define WINTC_HELLO_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_HELLO_WINDOW, WinTCHelloWindowClass))
#define IS_WINTC_HELLO_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_HELLO_WINDOW))
#define IS_WINTC_HELLO_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_HELLO_WINDOW))
#define WINTC_HELLO_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_HELLO_WINDOW, WinTCHelloWindowClass))

GType wintc_hello_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_hello_window_new(
    WinTCHelloApplication* app
);

#endif
