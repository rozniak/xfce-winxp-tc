#include <glib.h>
#include <pwd.h>
#include <sys/types.h>
#include <wintc/comgtk.h>

#include "deploy.h"
#include "oobeuser.h"
#include "xfconf.h"

//
// FORWARD DECLARATIONS
//
static gboolean wintc_oobe_user_is_eligible_user(
    struct passwd* pwent
);

//
// PUBLIC FUNCTIONS
//
gboolean wintc_oobe_user_apply_all(
    WINTC_UNUSED(GError** error)
)
{
    static const gchar* S_DEPLOYS_AUTOSTART[] = {
        "/uk/oddmatics/wintc/oobe/startup-desktop.desktop",
        "WinTC-Desktop.desktop",

        "/uk/oddmatics/wintc/oobe/startup-taskband.desktop",
        "WinTC-Taskband.desktop"
    };

    GError*        local_error = NULL;
    struct passwd* pwent       = NULL;

    while ((pwent = getpwent()))
    {
        if (!wintc_oobe_user_is_eligible_user(pwent))
        {
            WINTC_LOG_DEBUG(
                "oobe: skipping config on %s (%u)",
                pwent->pw_name,
                pwent->pw_uid
            );

            continue;
        }

        WINTC_LOG_DEBUG(
            "oobe: applying config for %s (%u)",
            pwent->pw_name,
            pwent->pw_uid
        );

        // XFCONF deployments
        //
        wintc_oobe_xfconf_update_channel(
            pwent->pw_dir,
            "xfwm4"
        );

        // File deployments
        //
        gchar* user_config_autostart =
            g_build_path(
                G_DIR_SEPARATOR_S,
                pwent->pw_dir,
                ".config",
                "autostart",
                NULL
            );

        for (gsize i = 0; i < G_N_ELEMENTS(S_DEPLOYS_AUTOSTART); i += 2)
        {
            if (
                !wintc_oobe_deploy_drop_file(
                    S_DEPLOYS_AUTOSTART[i],
                    user_config_autostart,
                    S_DEPLOYS_AUTOSTART[i + 1],
                    &local_error
                )
            )
            {
                wintc_log_error_and_clear(&local_error);
            }
        }

        g_free(user_config_autostart);
    }

    endpwent();

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_oobe_user_is_eligible_user(
    struct passwd* pwent
)
{
    static const gchar* k_hidden_users[] = {
        "nobody", "nobody4", "noaccess"
    };
    static const gchar* k_hidden_shells[] = {
        "/false", "/nologin"
    };

    //
    // These checks are the same as would normally be found in
    // /etc/lightdm/users.conf
    //
    gsize i;

    if (pwent->pw_uid < 500)
    {
        return FALSE;
    }

    for (i = 0; i < G_N_ELEMENTS(k_hidden_users); i++)
    {
        if (g_strcmp0(pwent->pw_name, k_hidden_users[i]) == 0)
        {
            return FALSE;
        }
    }

    for (i = 0; i < G_N_ELEMENTS(k_hidden_shells); i++)
    {
        if (g_str_has_suffix(pwent->pw_shell, k_hidden_shells[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}
