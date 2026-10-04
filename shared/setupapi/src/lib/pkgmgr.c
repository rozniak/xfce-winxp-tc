#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/pkgmgr.h"

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_PKG_FILE_EXT =
#if defined(WINTC_PKGMGR_APK)
    ".apk";
#elif defined(WINTC_PKGMGR_ARCHPKG)
    ".pkg.tar.zst";
#elif defined(WINTC_PKGMGR_BSDPKG)
    ".pkg";
#elif defined(WINTC_PKGMGR_DEB)
    ".deb";
#elif defined(WINTC_PKGMGR_RPM)
    ".rpm";
#elif defined(WINTC_PKGMGR_XBPS)
    ".xbps";
#endif

//
// PUBLIC FUNCTIONS
//
gchar* wintc_pkg_get_package_name(
    const gchar* path
)
{
    if (g_file_test(path, G_FILE_TEST_EXISTS))
    {
        return g_path_get_basename(path);
    }

    return g_strdup(path);
}

const gchar* wintc_pkg_true_package_name(
    const gchar* name
)
{
#if defined(WINTC_PKGMGR_DEB) || defined(WINTC_PKGMGR_APK)
    return name;
#else
    if (g_str_has_prefix(name, "lib"))
    {
        return name + 3;
    }

    return name;
#endif
}
