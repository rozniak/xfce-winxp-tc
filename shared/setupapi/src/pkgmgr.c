#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/pkgmgr.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_pkg_get_package_name(
    const gchar* path
)
{
    // FIXME: dpkg only
    //
    static gchar* s_argv_dpkg[] = {
        "/usr/bin/dpkg",
        "--field",
        NULL,
        "Package",
        NULL
    };

    s_argv_dpkg[2] = g_strdup(path); // FIXME: magic

    gchar*  cmd_out;
    GError* error = NULL;

    if (
        !g_spawn_sync(
            NULL,
            s_argv_dpkg,
            NULL,
            G_SPAWN_STDERR_TO_DEV_NULL,
            NULL,
            NULL,
            &cmd_out,
            NULL,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    g_free(s_argv_dpkg[2]);

    return cmd_out;
}
