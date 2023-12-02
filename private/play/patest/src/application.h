#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCPaTestApplicationClass WinTCPaTestApplicationClass;
typedef struct _WinTCPaTestApplication      WinTCPaTestApplication;

#define TYPE_WINTC_PATEST_APPLICATION            (wintc_patest_application_get_type())
#define WINTC_PATEST_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_PATEST_APPLICATION, WinTCPaTestApplication))
#define WINTC_PATEST_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_PATEST_APPLICATION, WinTCPaTestApplicationClass))
#define IS_WINTC_PATEST_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_PATEST_APPLICATION))
#define IS_WINTC_PATEST_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_PATEST_APPLICATION))
#define WINTC_PATEST_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_PATEST_APPLICATION, WinTCPaTestApplicationClass))

GType wintc_patest_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCPaTestApplication* wintc_patest_application_new(void);

#endif
