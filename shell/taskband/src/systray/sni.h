#ifndef __SYSTRAY_SNI_H__
#define __SYSTRAY_SNI_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIFICATION_SNI (wintc_notification_sni_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotificationSni,
    wintc_notification_sni,
    WINTC,
    NOTIFICATION_SNI,
    WinTCShextUIController
)

#endif
