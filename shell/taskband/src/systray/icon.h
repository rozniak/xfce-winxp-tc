#ifndef __SYSTRAY_ICON_H__
#define __SYSTRAY_ICON_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NOTIF_AREA_ICON (wintc_notif_area_icon_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNotifAreaIcon,
    wintc_notif_area_icon,
    WINTC,
    NOTIF_AREA_ICON,
    GtkWidget
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notif_area_icon_new(void);

const gchar* wintc_notif_area_icon_get_icon_name(
    WinTCNotifAreaIcon* notif_icon
);
void wintc_notif_area_icon_set_icon_name(
    WinTCNotifAreaIcon* notif_icon,
    const gchar*        icon_name
);

#endif
