#include <glib.h>
#include <pwd.h>
#include <sys/types.h>
#include <wintc/comgtk.h>

#include "oobeuser.h"

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
    struct passwd* pwent = NULL;

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
