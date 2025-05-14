#ifndef __PROCMON_H__
#define __PROCMON_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC ENUMS
//
enum
{
    COLUMN_PID        = 0,
    COLUMN_IMAGE_NAME,
    COLUMN_USER_NAME,
    COLUMN_CPU_TIME, // utime + stime
    COLUMN_CPU_USAGE,
    COLUMN_MEM_USAGE,
    N_COLUMNS
};

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TASKMGR_PROCMON (wintc_taskmgr_procmon_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCTaskmgrProcmon,
    wintc_taskmgr_procmon,
    WINTC,
    TASKMGR_PROCMON,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCTaskmgrProcmon* wintc_taskmgr_procmon_get_instance(void);

void wintc_taskmgr_procmon_bind_tree_view_model(
    WinTCTaskmgrProcmon* procmon,
    GtkTreeView*         tree_view
);

#endif
