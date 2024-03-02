#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCRunApplicationClass WinTCRunApplicationClass;
typedef struct _WinTCRunApplication      WinTCRunApplication;

#define WINTC_TYPE_RUN_APPLICATION            (wintc_run_application_get_type())
#define WINTC_RUN_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_RUN_APPLICATION, WinTCRunApplication))
#define WINTC_RUN_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_RUN_APPLICATION, WinTCRunApplicationClass))
#define IS_WINTC_RUN_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_RUN_APPLICATION))
#define IS_WINTC_RUN_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_RUN_APPLICATION))
#define WINTC_RUN_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_RUN_APPLICATION, WinTCRunApplicationClass))

GType wintc_run_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCRunApplication* wintc_run_application_new(void);

#endif
