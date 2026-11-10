#ifndef __NETSHELL_DIALOGS_H__
#define __NETSHELL_DIALOGS_H__

#include <gtk/gtk.h>

#include "device.h"

//
// PUBLIC FUNCTIONS
//
void wintc_net_repair_connection(
    WinTCNetDevice* device,
    const gchar*    connection_name,
    GtkWidget*      parent
);

#endif
