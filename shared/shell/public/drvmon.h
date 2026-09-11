#ifndef __SHELL_DRVMON_H__
#define __SHELL_DRVMON_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_DRIVE_MONITOR (wintc_sh_drive_monitor_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShDriveMonitor,
    wintc_sh_drive_monitor,
    WINTC,
    SH_DRIVE_MONITOR,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShDriveMonitor* wintc_sh_drive_monitor_get(
    WinTCShextHost* shext_host
);

gboolean wintc_sh_drive_monitor_get_path_is_mount(
    WinTCShDriveMonitor* drvmon,
    const gchar*         path
);

#endif
