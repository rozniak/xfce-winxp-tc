#ifndef __VOLUME_H__
#define __VOLUME_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotificationVolumeClass WinTCNotificationVolumeClass;
typedef struct _WinTCNotificationVolume      WinTCNotificationVolume;

#define WINTC_TYPE_NOTIFICATION_VOLUME            (wintc_notification_volume_get_type())
#define WINTC_NOTIFICATION_VOLUME(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTIFICATION_VOLUME, WinTCNotificationVolume))
#define WINTC_NOTIFICATION_VOLUME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationVolumeClass))
#define IS_WINTC_NOTIFICATION_VOLUME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTIFICATION_VOLUME))
#define IS_WINTC_NOTIFICATION_VOLUME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTIFICATION_VOLUME))
#define WINTC_NOTIFICATION_VOLUME_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTIFICATION_VOLUME, WinTCNotificationVolume))

GType wintc_notification_volume_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCNotificationVolume* wintc_notification_volume_new(
    GtkWidget* widget_notif
);

#endif
