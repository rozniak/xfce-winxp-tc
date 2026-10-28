#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "adrbar.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_shext_init(
    WinTCIShextShellWindow* shwnd,
    WINTC_UNUSED(GError** error)
)
{
    wintc_shext_ui_controller_new_from_type(
        WINTC_TYPE_TOOLBAR_ADDRESS,
        WINTC_ISHEXT_UI_HOST(shwnd)
    );

    return TRUE;
}
