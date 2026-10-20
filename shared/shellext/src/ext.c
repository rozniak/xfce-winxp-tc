#include <dlfcn.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/ext.h"

//
// PUBLIC FUNCTIONS
//
WinTCShextExtFunc wintc_shext_ext_get_api(
    const gchar* lib_path
)
{
    void*             dl_shext = NULL;
    WinTCShextExtFunc fn_shext = NULL;

    dl_shext = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);

    if (dl_shext)
    {
        fn_shext = (WinTCShextExtFunc) dlsym(dl_shext, "wintc_shext_init");

        if (!fn_shext)
        {
            g_warning(
                "shellext: unable to find wintc_shext_init in %s",
                lib_path
            );
        }
    }
    else
    {
        g_warning("shellext: unable to dlopen %s", lib_path);
    }

    return fn_shext;
}
