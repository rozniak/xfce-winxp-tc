#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCRegSvcApplicationClass WinTCRegSvcApplicationClass;
typedef struct _WinTCRegSvcApplication      WinTCRegSvcApplication;

#define WINTC_TYPE_REGSVC_APPLICATION            (wintc_regsvc_application_get_type())
#define WINTC_REGSVC_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_REGSVC_APPLICATION, WinTCRegSvcApplication))
#define WINTC_REGSVC_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_REGSVC_APPLICATION, WinTCRegSvcApplicationClass))
#define IS_WINTC_REGSVC_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_REGSVC_APPLICATION))
#define IS_WINTC_REGSVC_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_REGSVC_APPLICATION))
#define WINTC_REGSVC_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_REGSVC_APPLICATION, WinTCRegSvcApplicationClass))

GType wintc_regsvc_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCRegSvcApplication* wintc_regsvc_application_new(void);

#endif
