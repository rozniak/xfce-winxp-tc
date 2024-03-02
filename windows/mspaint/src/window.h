#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCPaintWindowClass WinTCPaintWindowClass;
typedef struct _WinTCPaintWindow      WinTCPaintWindow;

#define WINTC_TYPE_PAINT_WINDOW            (wintc_paint_window_get_type())
#define WINTC_PAINT_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_PAINT_WINDOW, WinTCPaintWindow))
#define WINTC_PAINT_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_PAINT_WINDOW, WinTCPaintWindowClass))
#define IS_WINTC_PAINT_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_PAINT_WINDOW))
#define IS_WINTC_PAINT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_PAINT_WINDOW))
#define WINTC_PAINT_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_PAINT_WINDOW, WinTCPaintWindowClass))

GType wintc_paint_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_paint_window_new(
    WinTCPaintApplication* app
);

#endif
