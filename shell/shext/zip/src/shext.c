#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "vwzip.h"

//
// FORWARD DECLARATIONS
//
static WinTCIShextView* factory_view_zip(
    WinTCShextHost*           shext_host,
    WinTCShextViewAssoc       assoc,
    const gchar*              assoc_str,
    const WinTCShextPathInfo* path_info
);

//
// PUBLIC FUNCTIONS
//
gboolean shext_init(
    WinTCShextHost* shext_host
)
{
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_use_view_for_mime(
            shext_host,
            "application/zip",
            factory_view_zip
        ),
        FALSE
    );

    return TRUE;
}

//
// CALLBACKS
//
static WinTCIShextView* factory_view_zip(
    WINTC_UNUSED(WinTCShextHost*     shext_host),
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    WINTC_UNUSED(const gchar*        assoc_str),
    const WinTCShextPathInfo* path_info
)
{
    WINTC_LOG_DEBUG("%s", "shext-zip: create new zip view");

    return wintc_view_zip_new(
        path_info->base_path,
        path_info->extended_path
    );
}
