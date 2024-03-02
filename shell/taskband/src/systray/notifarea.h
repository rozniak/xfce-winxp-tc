#ifndef __NOTIFAREA_H__
#define __NOTIFAREA_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotificationAreaClass WinTCNotificationAreaClass;
typedef struct _WinTCNotificationArea      WinTCNotificationArea;

#define WINTC_TYPE_NOTIFICATION_AREA            (wintc_notification_area_get_type())
#define WINTC_NOTIFICATION_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTIFICATION_AREA, WinTCNotificationArea))
#define WINTC_NOTIFICATION_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTIFICATION_AREA, WinTCNotificationArea))
#define IS_WINTC_NOTIFICATION_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTIFICATION_AREA))
#define IS_WINTC_NOTIFICATION_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTIFICATION_AREA))
#define WINTC_NOTIFICATION_AREA_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTIFICATION_AREA))

//
// PUBLIC FUNCTIONS
//
GtkWidget* notification_area_new(void);

#endif
