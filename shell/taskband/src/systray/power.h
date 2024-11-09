#ifndef __POWER_H__
#define __POWER_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotificationPowerClass WinTCNotificationPowerClass;
typedef struct _WinTCNotificationPower      WinTCNotificationPower;

#define WINTC_TYPE_NOTIFICATION_POWER            (wintc_notification_power_get_type())
#define WINTC_NOTIFICATION_POWER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTIFICATION_POWER, WinTCNotificationPower))
#define WINTC_NOTIFICATION_POWER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationPowerClass))
#define IS_WINTC_NOTIFICATION_POWER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTIFICATION_POWER))
#define IS_WINTC_NOTIFICATION_POWER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTIFICATION_POWER))
#define WINTC_NOTIFICATION_POWER_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTIFICATION_POWER, WinTCNotificationPower))

GType wintc_notification_power_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCNotificationPower* wintc_notification_power_new(
    GtkWidget* widget_notif
);

#endif
