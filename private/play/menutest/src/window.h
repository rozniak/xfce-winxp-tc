#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCMenuTestWindowClass WinTCMenuTestWindowClass;
typedef struct _WinTCMenuTestWindow      WinTCMenuTestWindow;

#define WINTC_TYPE_MENU_TEST_WINDOW            (wintc_menu_test_window_get_type())
#define WINTC_MENU_TEST_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_MENU_TEST_WINDOW, WinTCMenuTestWindow))
#define WINTC_MENU_TEST_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_MENU_TEST_WINDOW, WinTCMenuTestWindowClass))
#define IS_WINTC_MENU_TEST_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_MENU_TEST_WINDOW))
#define IS_WINTC_MENU_TEST_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_MENU_TEST_WINDOW))
#define WINTC_MENU_TEST_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_MENU_TEST_WINDOW, WinTCMenuTestWindowClass))

GType wintc_menu_test_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_menu_test_window_new(
    WinTCMenuTestApplication* app
);

#endif
