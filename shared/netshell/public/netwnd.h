#ifndef __NET_NETWND_H__
#define __NET_NETWND_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "device.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_WLAN_NETWORKS_WINDOW (wintc_wlan_networks_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCWlanNetworksWindow,
    wintc_wlan_networks_window,
    WINTC,
    WLAN_NETWORKS_WINDOW,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wlan_networks_window_new(
    WinTCNetDevice* device,
    const gchar*    connection_name
);

#endif
