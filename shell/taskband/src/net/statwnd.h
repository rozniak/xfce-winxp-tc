#ifndef __NET_STATWND_H__
#define __NET_STATWND_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "device.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NET_STATUS_WINDOW (wintc_net_status_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNetStatusWindow,
    wintc_net_status_window,
    WINTC,
    NET_STATUS_WINDOW,
    GtkWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_net_status_window_new(
    WinTCNetDevice* device,
    const gchar*    connection_name
);

#endif
