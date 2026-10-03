#include <alpm.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/error.h"
#include "pkgcmn.h"

//
// PUBLIC FUNCTIONS
//
void wintc_pkg_alpm_error_set(
    GError**     error,
    alpm_errno_t error_code
)
{
    g_set_error(
        error,
        WINTC_SETUPAPI_ERROR,
        WINTC_SETUPAPI_ERROR_FAILED,
        "%s",
        alpm_strerror(error_code)
    );
}

alpm_handle_t* wintc_pkg_alpm_handle_get(
    GError** error
)
{
    alpm_errno_t   error_code;
    alpm_handle_t* handle =
        alpm_initialize("/", "/var/lib/pacman", &error_code);

    if (!handle)
    {
        wintc_pkg_alpm_error_set(
            error,
            error_code
        );
    }

    return NULL;
}
