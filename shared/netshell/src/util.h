#ifndef __NETSHELL_UTIL_H__
#define __NETSHELL_UTIL_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../public/device.h"

//
// PUBLIC DEFINES
//

//
// PUBLIC FUNCTIONS
//
void wintc_net_ensure_styles(void);

GtkWidget* wintc_wlan_create_signal_image(void);
void wintc_wlan_signal_image_set_signal(
    GtkWidget* image,
    gint       signal
);

#endif
