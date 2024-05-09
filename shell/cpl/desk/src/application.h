#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplDeskApplicationClass WinTCCplDeskApplicationClass;
typedef struct _WinTCCplDeskApplication      WinTCCplDeskApplication;

#define WINTC_TYPE_CPL_DESK_APPLICATION            (wintc_cpl_desk_application_get_type())
#define WINTC_CPL_DESK_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_DESK_APPLICATION, WinTCCplDeskApplication))
#define WINTC_CPL_DESK_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_DESK_APPLICATION, WinTCCplDeskApplicationClass))
#define IS_WINTC_CPL_DESK_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_DESK_APPLICATION))
#define IS_WINTC_CPL_DESK_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_DESK_APPLICATION))
#define WINTC_CPL_DESK_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CPL_DESK_APPLICATION, WinTCCplDeskApplicationClass))

GType wintc_cpl_desk_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCCplDeskApplication* wintc_cpl_desk_application_new(void);

#endif
