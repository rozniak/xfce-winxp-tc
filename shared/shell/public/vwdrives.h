#ifndef __SHELL_VWDRIVES_H__
#define __SHELL_VWDRIVES_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_VIEW_DRIVES (wintc_sh_view_drives_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShViewDrives,
    wintc_sh_view_drives,
    WINTC,
    SH_VIEW_DRIVES,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_drives_new(
    WinTCShextHost* shext_host
);

#endif
