#include <glib.h>
#include <wintc/comgtk.h>

#include "../../public/pkgmgr.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_pkg_get_package_name(
    const gchar* path
)
{
    if (g_file_test(path, G_FILE_TEST_EXISTS))
    {
        return g_strdup(path);
    }

    return g_path_get_basename(path);
}
