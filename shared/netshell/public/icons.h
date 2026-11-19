#ifndef __NETSHELL_ICONS_H__
#define __NETSHELL_ICONS_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "device.h"

//
// PUBLIC FUNCTIONS
//
const gchar* const* wintc_net_get_activity_icon_names(
    WinTCNetActivity activity
);
const gchar* const* wintc_net_get_tray_icon_names(
    WinTCNetDeviceKind kind,
    WinTCNetState      state,
    WinTCNetActivity   activity,
    guint              frame
);
const gchar* const* wintc_wlan_get_preferred_icon_names(void);
const gchar* const* wintc_wlan_get_secured_icon_names(void);
const gchar* const* wintc_wlan_get_signal_icon_names(
    gint signal
);

GdkPixbuf* wintc_net_load_icon(
    const gchar* const* icon_names,
    gint                size
);
void wintc_net_image_set_icon_names(
    GtkImage*           image,
    const gchar* const* icon_names,
    gint                size
);

#endif
