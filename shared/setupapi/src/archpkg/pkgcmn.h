#ifndef __SETUPAPI_PKGCMN_H__
#define __SETUPAPI_PKGCMN_H__

#include <alpm.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//
void wintc_pkg_alpm_error_set(
    GError**     error,
    alpm_errno_t error_code
);

alpm_handle_t* wintc_pkg_alpm_handle_get(
    GError** error
);

#endif
