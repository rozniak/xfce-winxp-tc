#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCWinverApplicationClass WinTCWinverApplicationClass;
typedef struct _WinTCWinverApplication      WinTCWinverApplication;

#define WINTC_TYPE_WINVER_APPLICATION            (wintc_winver_application_get_type())
#define WINTC_WINVER_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_WINVER_APPLICATION, WinTCWinverApplication))
#define WINTC_WINVER_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_WINVER_APPLICATION, WinTCWinverApplicationClass))
#define IS_WINTC_WINVER_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_WINVER_APPLICATION))
#define IS_WINTC_WINVER_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_WINVER_APPLICATION))
#define WINTC_WINVER_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_WINVER_APPLICATION, WinTCWinverApplicationClass))

GType wintc_winver_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCWinverApplication* wintc_winver_application_new(void);

#endif
