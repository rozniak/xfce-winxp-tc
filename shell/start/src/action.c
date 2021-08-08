#include <gdk/gdk.h>
#include <glib.h>

#include "action.h"
#include "util.h"

//
// FORWARD DECLARATIONS
//
static void set_display(
    const gchar* display
);

//
// PUBLIC FUNCTIONS
//
void launch_action(
    gint action_id
)
{
    gchar**      argv;
    const gchar* cmdline;

    switch (action_id)
    {
        case XP_ACTION_MYDOCS:
            cmdline = "exo-open --launch FileManager Documents";
            break;

        case XP_ACTION_MYPICS:
            cmdline = "exo-open --launch FileManager Pictures";
            break;

        case XP_ACTION_MYMUSIC:
            cmdline = "exo-open --launch FileManager Music";
            break;

        // TODO: There isn't really a good option for this at the moment, maybe if we
        //       implement an Explorer replica we can have it open a view of disks
        //
        case XP_ACTION_MYCOMP:
            cmdline = "exo-open --launch FileManager /";
            break;

        case XP_ACTION_CONTROL:
            cmdline = "xfce4-settings-manager";
            break;

        case XP_ACTION_MIMEMGMT:
            cmdline = "xfce4-mime-settings";
            break;

        case XP_ACTION_PRINTERS:
            cmdline = "system-config-printer";
            break;

        // TODO: One day write our own log off and shut down XP dialogs, and execute
        //       them here
        //
        case XP_ACTION_LOGOFF:
        case XP_ACTION_SHUTDOWN:
            cmdline = "xfce4-session-logout";
            break;

        // Default to a 'sorry, not implemented' message
        //
        default:
            display_not_implemented_error();
            return;
    }

    argv = true_shell_parse_argv(cmdline);

    launch_command(argv);

    g_strfreev(argv);
}

void launch_command(
    gchar** command
)
{
    if (command == NULL)
    {
        g_error("There is no argument vector to execute.");
        return;
    }

    gchar* display =
        g_strdup(
            gdk_display_get_name(gdk_display_get_default())
        );

    g_spawn_async(
        NULL,
        command,
        NULL,
        0,
        (GSpawnChildSetupFunc) set_display,
        display,
        NULL,
        NULL
    );

    g_free(display);
}

//
// HELPERS
//
static void set_display(
    const gchar* display
)
{
    g_setenv("DISPLAY", display, TRUE);
}
