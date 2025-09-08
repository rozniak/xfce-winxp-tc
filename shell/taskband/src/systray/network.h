#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIFICATION_NETWORK (wintc_notification_network_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotificationNetwork,
    wintc_notification_network,
    WINTC,
    NOTIFICATION_NETWORK,
    WinTCShextUIController
)

#endif
