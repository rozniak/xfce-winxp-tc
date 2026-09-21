#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSysAnalApplicationClass WinTCSysAnalApplicationClass;
typedef struct _WinTCSysAnalApplication      WinTCSysAnalApplication;

#define WINTC_TYPE_SYS_ANAL_APPLICATION            (wintc_sys_anal_application_get_type())
#define WINTC_SYS_ANAL_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SYS_ANAL_APPLICATION, WinTCSysAnalApplication))
#define WINTC_SYS_ANAL_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SYS_ANAL_APPLICATION, WinTCSysAnalApplicationClass))
#define IS_WINTC_SYS_ANAL_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SYS_ANAL_APPLICATION))
#define IS_WINTC_SYS_ANAL_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SYS_ANAL_APPLICATION))
#define WINTC_SYS_ANAL_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SYS_ANAL_APPLICATION, WinTCSysAnalApplicationClass))

GType wintc_sys_anal_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCSysAnalApplication* wintc_sys_anal_application_new(void);

#endif
