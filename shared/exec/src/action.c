#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/action.h"
#include "../public/errors.h"
#include "../public/exec.h"

//
// FORWARD DECLARATIONS
//
gboolean launch_exo_fm(
    const gchar* target,
    GError**     error
);

//
// PUBLIC FUNCTIONS
//
gboolean wintc_launch_action(
    WinTCAction action_id,
    GError**    error
)
{
    switch (action_id)
    {
        case WINTC_ACTION_MYDOCS:
            return
                launch_exo_fm(
                    g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),
                    error
                );

        case WINTC_ACTION_MYPICS:
            return
                launch_exo_fm(
                    g_get_user_special_dir(G_USER_DIRECTORY_PICTURES),
                    error
                );

        case WINTC_ACTION_MYMUSIC:
            return
                launch_exo_fm(
                    g_get_user_special_dir(G_USER_DIRECTORY_MUSIC),
                    error
                );

        // TODO: There isn't really a good option for this at the moment, maybe if we
        //       implement an Explorer replica we can have it open a view of disks
        //
        case WINTC_ACTION_MYCOMP:
            return
                launch_exo_fm(
                    "/",
                    error
                );

        case WINTC_ACTION_CONTROL:
            return wintc_launch_command("xfce4-settings-manager", error);

        case WINTC_ACTION_MIMEMGMT:
            return wintc_launch_command("xfce4-mime-settings", error);

        case WINTC_ACTION_PRINTERS:
            return wintc_launch_command("system-config-printer", error);

        case WINTC_ACTION_RUN:
            // TODO: In future we will handle missing components gracefully (say, if
            //       the Run component is missing here)
            //
            //       See issue #134 for details
            //
            return wintc_launch_command("run", error);

        case WINTC_ACTION_LOGOFF:
            return
                wintc_launch_command_with_fallbacks(
                    error,
                    "wintc-exitwin --user-options",
                    "xfce4-session-logout",
                    NULL
                );

        case WINTC_ACTION_SHUTDOWN:
            return
                wintc_launch_command_with_fallbacks(
                    error,
                    "wintc-exitwin --power-options",
                    "xfce4-session-logout",
                    NULL
                );

        // Default to 'not implemented' error
        //
        default:
            g_set_error(
                error,
                WINTC_GENERAL_ERROR,
                WINTC_GENERAL_ERROR_NOTIMPL,
                "Action code is not implemented: %d",
                action_id
            );

            return FALSE;
    }
}

//
// PRIVATE FUNCTIONS
//
gboolean launch_exo_fm(
    const gchar* target,
    GError**     error
)
{
    gchar*   cmdline;
    gboolean ret;

    cmdline =
        g_strdup_printf(
            "exo-open --launch FileManager %s",
            target
        );

    ret = wintc_launch_command(cmdline, error);

    g_free(cmdline);

    return ret;
}
