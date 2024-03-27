#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "shext.h"
#include "vwprntrs.h"

//
// FORWARD DECLARATIONS
//
static WinTCIShextView* factory_view_cpl_printers(
    WinTCShextViewAssoc assoc,
    const gchar*        assoc_str,
    const gchar*        url
);

//
// PUBLIC FUNCTIONS
//
gboolean shext_init(
    WinTCShextHost* shext_host
)
{
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_view(
            shext_host,
            WINTC_SH_GUID_PRINTERS,
            factory_view_cpl_printers
        ),
        FALSE
    );

    return TRUE;
}

//
// CALLBACKS
//
static WinTCIShextView* factory_view_cpl_printers(
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    WINTC_UNUSED(const gchar*        assoc_str),
    WINTC_UNUSED(const gchar*        url)
)
{
    WINTC_LOG_DEBUG("%s", "cpl-prntrs: create new cpl printers view");

    return wintc_cpl_view_printers_new();
}
