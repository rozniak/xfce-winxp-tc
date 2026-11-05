#ifndef __NET_DIALOGS_H__
#define __NET_DIALOGS_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "device.h"

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wlan_key_dialog_new(
    GtkWindow*        parent,
    const gchar*      title,
    const gchar*      ssid,
    WinTCWlanSecurity security
);
const gchar* wintc_wlan_key_dialog_get_key(
    GtkWidget* dialog
);

GtkWidget* wintc_wlan_progress_dialog_new(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* ssid
);
void wintc_wlan_progress_dialog_update(
    GtkWidget*      dialog,
    WinTCNetDevice* device
);
void wintc_wlan_progress_dialog_set_connected(
    GtkWidget* dialog
);

GtkWidget* wintc_wlan_unsecured_dialog_new(
    GtkWindow*   parent,
    const gchar* title,
    const gchar* ssid
);

void wintc_net_repair_connection(
    WinTCNetDevice* device,
    const gchar*    connection_name,
    GtkWidget*      parent
);

#endif
