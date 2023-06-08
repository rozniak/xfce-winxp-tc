#include <gdk/gdk.h>
#include <glib.h>
#include <wintc-comgtk.h>
#include <wintc-exec.h>

#include "action.h"

//
// PUBLIC FUNCTIONS
//
void launch_action(
    WinTCAction action_id
)
{
    GError* error = NULL;

    if (!wintc_launch_action(action_id, &error))
    {
        wintc_nice_error_and_clear(&error);
    }
}

void launch_command(
    const gchar* command
)
{
    if (command == NULL)
    {
        g_error("No command specified.");
        return;
    }

    GError* error = NULL;

    if (!wintc_launch_command(command, &error))
    {
        wintc_nice_error_and_clear(&error);
    }
}
