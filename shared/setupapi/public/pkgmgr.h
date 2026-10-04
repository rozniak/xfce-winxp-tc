#ifndef __SETUPAPI_PKGMGR_H__
#define __SETUPAPI_PKGMGR_H__

#include <glib.h>

//
// PUBLIC CONSTANTS
//
extern const gchar* WINTC_PKG_FILE_EXT;

//
// PUBLIC FUNCTIONS
//
gchar* wintc_pkg_get_package_name(
    const gchar* path
);

#endif
