#ifndef __SHCOMMON_MONITOR_H__
#define __SHCOMMON_MONITOR_H__

#include <gio/gio.h>
#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_DIR_MONITOR_RECURSIVE (wintc_sh_dir_monitor_recursive_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShDirMonitorRecursive,
    wintc_sh_dir_monitor_recursive,
    WINTC,
    SH_DIR_MONITOR_RECURSIVE,
    GObject
)
    

//
// PUBLIC FUNCTIONS
//
WinTCShDirMonitorRecursive* wintc_sh_fs_monitor_directory_recursive(
    GFile*            file,
    GFileMonitorFlags flags,
    GCancellable*     cancellable,
    GError**          error
);

#endif
