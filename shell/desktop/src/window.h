#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDesktopWindowClass WinTCDesktopWindowClass;
typedef struct _WinTCDesktopWindow      WinTCDesktopWindow;

#define TYPE_WINTC_DESKTOP_WINDOW            (wintc_desktop_window_get_type())
#define WINTC_DESKTOP_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_DESKTOP_WINDOW, WinTCDesktopWindow))
#define WINTC_DESKTOP_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_DESKTOP_WINDOW, WinTCDesktopWindow))
#define IS_WINTC_DESKTOP_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_DESKTOP_WINDOW))
#define IS_WINTC_DESKTOP_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_DESKTOP_WINDOW))
#define WINTC_DESKTOP_WINDOW_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_DESKTOP_WINDOW))

GType wintc_desktop_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTION
//
GtkWidget* wintc_desktop_window_new(
        WinTCDesktopApplication* app
);

#endif
