#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

#include "../public/tool.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_net_tool_exists(
    const gchar* program
)
{
    gchar* path = g_find_program_in_path(program);

    if (!path)
    {
        return FALSE;
    }

    g_free(path);

    return TRUE;
}

void wintc_net_launch_tool(
    const gchar* cmdline,
    GtkWidget*   widget
)
{
    GError* error = NULL;

    if (!wintc_launch_command(cmdline, &error))
    {
        wintc_display_error_and_clear(
            &error,
            widget ? wintc_widget_get_toplevel_window(widget) : NULL
        );
    }
}
