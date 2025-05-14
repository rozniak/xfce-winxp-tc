#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCTaskmgrApplicationClass WinTCTaskmgrApplicationClass;
typedef struct _WinTCTaskmgrApplication      WinTCTaskmgrApplication;

#define WINTC_TYPE_TASKMGR_APPLICATION            (wintc_taskmgr_application_get_type())
#define WINTC_TASKMGR_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_TASKMGR_APPLICATION, WinTCTaskmgrApplication))
#define WINTC_TASKMGR_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_TASKMGR_APPLICATION, WinTCTaskmgrApplicationClass))
#define IS_WINTC_TASKMGR_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_TASKMGR_APPLICATION))
#define IS_WINTC_TASKMGR_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_TASKMGR_APPLICATION))
#define WINTC_TASKMGR_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_TASKMGR_APPLICATION, WinTCTaskmgrApplicationClass))

GType wintc_taskmgr_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCTaskmgrApplication* wintc_taskmgr_application_new(void);

#endif
