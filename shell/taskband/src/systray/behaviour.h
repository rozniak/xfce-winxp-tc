#ifndef __BEHAVIOUR_H__
#define __BEHAVIOUR_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCNotificationBehaviourClass
{
    GObjectClass __parent__;
} WinTCNotificationBehaviourClass;

typedef struct _WinTCNotificationBehaviour
{
    GObject __parent__;

    gchar*     icon_name;
    GtkWidget* widget_notif;
} WinTCNotificationBehaviour;

#define WINTC_TYPE_NOTIFICATION_BEHAVIOUR            (wintc_notification_behaviour_get_type())
#define WINTC_NOTIFICATION_BEHAVIOUR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationBehaviour))
#define WINTC_NOTIFICATION_BEHAVIOUR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationBehaviourClass))
#define IS_WINTC_NOTIFICATION_BEHAVIOUR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_NOTIFICATION_BEHAVIOUR))
#define IS_WINTC_NOTIFICATION_BEHAVIOUR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_NOTIFICATION_BEHAVIOUR))
#define WINTC_NOTIFICATION_BEHAVIOUR_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_NOTIFICATION_BEHAVIOUR, WinTCNotificationBehaviour))

GType wintc_notification_behaviour_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_notification_behaviour_get_icon_name(
    WinTCNotificationBehaviour* behaviour
);

#endif
