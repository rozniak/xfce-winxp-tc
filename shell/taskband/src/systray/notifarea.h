#ifndef __NOTIFAREA_H__
#define __NOTIFAREA_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIFICATION_AREA (wintc_notification_area_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotificationArea,
    wintc_notification_area,
    WINTC,
    NOTIFICATION_AREA,
    GtkBin
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* notification_area_new(void);

#endif
