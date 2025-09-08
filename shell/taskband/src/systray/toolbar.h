#ifndef __TOOLBAR_NOTIF_AREA_H__
#define __TOOLBAR_NOTIF_AREA_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_NOTIF_AREA (wintc_toolbar_notif_area_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarNotifArea,
    wintc_toolbar_notif_area,
    WINTC,
    TOOLBAR_NOTIF_AREA,
    WinTCShextUIController
)

#endif

