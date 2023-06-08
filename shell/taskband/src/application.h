#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCTaskbandApplicationClass WinTCTaskbandApplicationClass;
typedef struct _WinTCTaskbandApplication      WinTCTaskbandApplication;

#define TYPE_WINTC_TASKBAND_APPLICATION            (wintc_taskband_application_get_type())
#define WINTC_TASKBAND_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_TASKBAND_APPLICATION, WinTCTaskbandApplication))
#define WINTC_TASKBAND_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_TASKBAND_APPLICATION, WinTCTaskbandApplicationClass))
#define IS_WINTC_TASKBAND_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_TASKBAND_APPLICATION))
#define IS_WINTC_TASKBAND_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_TASKBAND_APPLICATION))
#define WINTC_TASKBAND_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_TASKBAND_APPLICATION, WinTCTaskbandApplication))

GType wintc_taskband_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCTaskbandApplication* wintc_taskband_application_new(void);

#endif
