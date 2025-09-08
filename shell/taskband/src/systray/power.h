#ifndef __POWER_H__
#define __POWER_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIFICATION_POWER (wintc_notification_power_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotificationPower,
    wintc_notification_power,
    WINTC,
    NOTIFICATION_POWER,
    WinTCShextUIController
)

#endif
