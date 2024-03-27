#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplSysdmApplicationClass WinTCCplSysdmApplicationClass;
typedef struct _WinTCCplSysdmApplication      WinTCCplSysdmApplication;

#define WINTC_TYPE_CPL_SYSDM_APPLICATION            (wintc_cpl_sysdm_application_get_type())
#define WINTC_CPL_SYSDM_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_SYSDM_APPLICATION, WinTCCplSysdmApplication))
#define WINTC_CPL_SYSDM_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_SYSDM_APPLICATION, WinTCCplSysdmApplicationClass))
#define IS_WINTC_CPL_SYSDM_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_SYSDM_APPLICATION))
#define IS_WINTC_CPL_SYSDM_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_SYSDM_APPLICATION))
#define WINTC_CPL_SYSDM_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CPL_SYSDM_APPLICATION, WinTCCplSysdmApplicationClass))

GType wintc_cpl_sysdm_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCCplSysdmApplication* wintc_cpl_sysdm_application_new(void);

#endif
