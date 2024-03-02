#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCPaTestWindowClass WinTCPaTestWindowClass;
typedef struct _WinTCPaTestWindow      WinTCPaTestWindow;

#define WINTC_TYPE_PATEST_WINDOW            (wintc_patest_window_get_type())
#define WINTC_PATEST_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_PATEST_WINDOW, WinTCPaTestWindow))
#define WINTC_PATEST_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_PATEST_WINDOW, WinTCPaTestWindowClass))
#define IS_WINTC_PATEST_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_PATEST_WINDOW))
#define IS_WINTC_PATEST_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_PATEST_WINDOW))
#define WINTC_PATEST_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_PATEST_WINDOW, WinTCPaTestWindowClass))

GType wintc_patest_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_patest_window_new(
    WinTCPaTestApplication* app
);

#endif
