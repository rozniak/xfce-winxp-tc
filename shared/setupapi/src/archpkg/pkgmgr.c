#include <alpm.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/pkgmgr.h"
#include "pkgcmn.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_pkg_get_package_name(
    const gchar* path
)
{
    GError*        error  = NULL;
    alpm_handle_t* handle = wintc_pkg_alpm_handle_get(&error);

    if (!handle)
    {
        wintc_log_error_and_clear(&error);

        return g_strdup("Unknown package");
    }

    // Read in package
    //
    alpm_pkg_t* package = NULL;
    gchar*      ret     = NULL;

    if (
        alpm_pkg_load(
            handle,
            path,
            1,
            ALPM_SIG_USE_DEFAULT,
            &package
        ) == 0
    )
    {
        ret = g_strdup(alpm_pkg_get_name(package));

        alpm_pkg_free(package);
    }
    else
    {
        g_critical(
            "setupapi: unable to load package via ALPM: %s, error: %s",
            path,
            alpm_strerror(alpm_errno(handle))
        );
    }

    alpm_release(handle);

    return ret;
}
