#ifndef __NETSHELL_UTIL_H__
#define __NETSHELL_UTIL_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "device.h"

//
// PUBLIC DEFINES
//
#define WINTC_NET_ACQUIRING_FRAMES 3
#define WINTC_WLAN_SIGNAL_BARS_MAX 5

//
// PUBLIC FUNCTIONS
//

void wintc_net_ensure_styles(void);

GtkWidget* wintc_wlan_create_signal_image(void);
void wintc_wlan_signal_image_set_signal(
    GtkWidget* image,
    gint       signal
);

gboolean wintc_net_tool_exists(
    const gchar* program
);
void wintc_net_launch_tool(
    const gchar* cmdline,
    GtkWidget*   widget
);

void wintc_wlan_network_free(
    WinTCWlanNetwork* network
);
gint wintc_wlan_network_compare(
    gconstpointer a,
    gconstpointer b
);

#endif
