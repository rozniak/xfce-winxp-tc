#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDesktopApplicationClass WinTCDesktopApplicationClass;
typedef struct _WinTCDesktopApplication      WinTCDesktopApplication;

#define TYPE_WINTC_DESKTOP_APPLICATION            (wintc_desktop_application_get_type())
#define WINTC_DESKTOP_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_DESKTOP_APPLICATION, WinTCDesktopApplication))
#define WINTC_DESKTOP_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_DESKTOP_APPLICATION, WinTCDesktopApplicationClass))
#define IS_WINTC_DESKTOP_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_DESKTOP_APPLICATION))
#define IS_WINTC_DESKTOP_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_DESKTOP_APPLICATION))
#define WINTC_DESKTOP_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_DESKTOP_APPLICATION, WinTCDesktopApplicationClass))

GType wintc_desktop_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCDesktopApplication* wintc_desktop_application_new(void);

#endif
