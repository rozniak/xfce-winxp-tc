#include <glib.h>
#include <wintc/comgtk.h>

#include "mfu.h"

//
// PUBLIC FUNCTIONS
//
void wintc_toolbar_start_mfu_bump_entry(
    const gchar* entry_name
)
{
    WINTC_LOG_DEBUG("MFU - ping for %s", entry_name);
}
