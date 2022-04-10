#include <glib.h>
#include <wintc-comgtk.h>

#include "action.h"
#include "exec.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_launch_action(
    WinTCAction action_id,
    GError**    out_error
)
{
    const gchar* cmdline;

    switch (action_id)
    {
        case WINTC_ACTION_MYDOCS:
            cmdline = "exo-open --launch FileManager Documents";
            break;

        case WINTC_ACTION_MYPICS:
            cmdline = "exo-open --launch FileManager Pictures";
            break;

        case WINTC_ACTION_MYMUSIC:
            cmdline = "exo-open --launch FileManager Music";
            break;

        // TODO: There isn't really a good option for this at the moment, maybe if we
        //       implement an Explorer replica we can have it open a view of disks
        //
        case WINTC_ACTION_MYCOMP:
            cmdline = "exo-open --launch FileManager /";
            break;

        case WINTC_ACTION_CONTROL:
            cmdline = "xfce4-settings-manager";
            break;

        case WINTC_ACTION_MIMEMGMT:
            cmdline = "xfce4-mime-settings";
            break;

        case WINTC_ACTION_PRINTERS:
            cmdline = "system-config-printer";
            break;

        case WINTC_ACTION_RUN:
            // TODO: In future we will handle missing components gracefully (say, if
            //       the Run component is missing here)
            //
            //       See issue #134 for details
            //
            cmdline = "run";
            break;

        // TODO: One day write our own log off and shut down dialogs, and execute
        //       them here
        //
        case WINTC_ACTION_LOGOFF:
        case WINTC_ACTION_SHUTDOWN:
            cmdline = "xfce4-session-logout";
            break;

        // Default to 'not implemented' error
        //
        default:
            g_set_error(
                out_error,
                WINTC_GENERAL_ERROR,
                WINTC_GENERAL_ERROR_NOTIMPL,
                "Action code is not implemented: %d",
                action_id
            );

            return FALSE;
    }

    return wintc_launch_command(cmdline, out_error);
}
