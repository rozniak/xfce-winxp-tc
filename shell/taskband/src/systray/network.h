#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotificationNetworkClass WinTCNotificationNetworkClass;
typedef struct _WinTCNotificationNetwork      WinTCNotificationNetwork;

#define WINTC_TYPE_NOTIFICATION_NETWORK            (wintc_notification_network_get_type())
#define WINTC_NOTIFICATION_NETWORK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTIFICATION_NETWORK, WinTCNotificationNetwork))
#define WINTC_NOTIFICATION_NETWORK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationNetworkClass))
#define IS_WINTC_NOTIFICATION_NETWORK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTIFICATION_NETWORK))
#define IS_WINTC_NOTIFICATION_NETWORK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTIFICATION_NETWORK))
#define WINTC_NOTIFICATION_NETWORK_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTIFICATION_NETWORK, WinTCNotificationNetwork))

GType wintc_notification_network_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCNotificationNetwork* wintc_notification_network_new(
    GtkWidget* widget_notif
);

#endif
