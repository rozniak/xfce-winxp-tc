#ifndef __NETSHELL_TOOL_H__
#define __NETSHELL_TOOL_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
gboolean wintc_net_tool_exists(
    const gchar* program
);
void wintc_net_launch_tool(
    const gchar* cmdline,
    GtkWidget*   widget
);

#endif
