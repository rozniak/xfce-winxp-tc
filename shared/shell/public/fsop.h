#ifndef __SHELL_FSOP_H__
#define __SHELL_FSOP_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SH_FS_OPERATION_INVALID,
    WINTC_SH_FS_OPERATION_COPY,
    WINTC_SH_FS_OPERATION_MOVE,
    WINTC_SH_FS_OPERATION_TRASH
} WinTCShFSOperationKind;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_FS_OPERATION (wintc_sh_fs_operation_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShFSOperation,
    wintc_sh_fs_operation,
    WINTC,
    SH_FS_OPERATION,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShFSOperation* wintc_sh_fs_operation_new(
    GList*                 files,
    const gchar*           dest,
    WinTCShFSOperationKind operation_kind
);

void wintc_sh_fs_operation_do(
    WinTCShFSOperation* fs_operation,
    GtkWindow*          wnd
);

#endif
