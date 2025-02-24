#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCHelloApplicationClass WinTCHelloApplicationClass;
typedef struct _WinTCHelloApplication      WinTCHelloApplication;

#define WINTC_TYPE_HELLO_APPLICATION            (wintc_hello_application_get_type())
#define WINTC_HELLO_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_HELLO_APPLICATION, WinTCHelloApplication))
#define WINTC_HELLO_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_HELLO_APPLICATION, WinTCHelloApplicationClass))
#define IS_WINTC_HELLO_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_HELLO_APPLICATION))
#define IS_WINTC_HELLO_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_HELLO_APPLICATION))
#define WINTC_HELLO_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_HELLO_APPLICATION, WinTCHelloApplicationClass))

GType wintc_hello_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCHelloApplication* wintc_hello_application_new(void);

#endif
