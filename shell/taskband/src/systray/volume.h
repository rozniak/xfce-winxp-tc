#ifndef __VOLUME_H__
#define __VOLUME_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIFICATION_VOLUME (wintc_notification_volume_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotificationVolume,
    wintc_notification_volume,
    WINTC,
    NOTIFICATION_VOLUME,
    WinTCShextUIController
)

#endif
