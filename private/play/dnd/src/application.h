#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDndTestApplicationClass WinTCDndTestApplicationClass;
typedef struct _WinTCDndTestApplication      WinTCDndTestApplication;

#define WINTC_TYPE_DND_TEST_APPLICATION            (wintc_dnd_test_application_get_type())
#define WINTC_DND_TEST_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_DND_TEST_APPLICATION, WinTCDndTestApplication))
#define WINTC_DND_TEST_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_DND_TEST_APPLICATION, WinTCDndTestApplicationClass))
#define IS_WINTC_DND_TEST_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_DND_TEST_APPLICATION))
#define IS_WINTC_DND_TEST_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_DND_TEST_APPLICATION))
#define WINTC_DND_TEST_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_DND_TEST_APPLICATION, WinTCDndTestApplicationClass))

GType wintc_dnd_test_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCDndTestApplication* wintc_dnd_test_application_new(void);

#endif
