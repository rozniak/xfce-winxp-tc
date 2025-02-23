#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCMenuTestApplicationClass WinTCMenuTestApplicationClass;
typedef struct _WinTCMenuTestApplication      WinTCMenuTestApplication;

#define WINTC_TYPE_MENU_TEST_APPLICATION            (wintc_menu_test_application_get_type())
#define WINTC_MENU_TEST_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_MENU_TEST_APPLICATION, WinTCMenuTestApplication))
#define WINTC_MENU_TEST_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_MENU_TEST_APPLICATION, WinTCMenuTestApplicationClass))
#define IS_WINTC_MENU_TEST_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_MENU_TEST_APPLICATION))
#define IS_WINTC_MENU_TEST_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_MENU_TEST_APPLICATION))
#define WINTC_MENU_TEST_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_MENU_TEST_APPLICATION, WinTCMenuTestApplicationClass))

GType wintc_menu_test_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCMenuTestApplication* wintc_menu_test_application_new(void);

#endif
