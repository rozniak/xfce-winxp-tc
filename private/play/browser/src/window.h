#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCBrowserWindowClass WinTCBrowserWindowClass;
typedef struct _WinTCBrowserWindow      WinTCBrowserWindow;

#define WINTC_TYPE_BROWSER_WINDOW            (wintc_browser_window_get_type())
#define WINTC_BROWSER_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_BROWSER_WINDOW, WinTCBrowserWindow))
#define WINTC_BROWSER_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_BROWSER_WINDOW, WinTCBrowserWindowClass))
#define IS_WINTC_BROWSER_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_BROWSER_WINDOW))
#define IS_WINTC_BROWSER_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_BROWSER_WINDOW))
#define WINTC_BROWSER_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_BROWSER_WINDOW, WinTCBrowserWindowClass))

GType wintc_browser_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_browser_window_new(
    WinTCBrowserApplication* app
);

#endif
