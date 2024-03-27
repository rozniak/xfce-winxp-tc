#ifndef __SHELL_VWFS_H__
#define __SHELL_VWFS_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShViewFSClass WinTCShViewFSClass;
typedef struct _WinTCShViewFS      WinTCShViewFS;

#define WINTC_TYPE_SH_VIEW_FS            (wintc_sh_view_fs_get_type())
#define WINTC_SH_VIEW_FS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_VIEW_FS, WinTCShViewFS))
#define WINTC_SH_VIEW_FS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_VIEW_FS, WinTCShViewFSClass))
#define IS_WINTC_SH_VIEW_FS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_VIEW_FS))
#define IS_WINTC_SH_VIEW_FS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_VIEW_FS))
#define WINTC_SH_VIEW_FS_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_SH_VIEW_FS, WinTCShViewFS))

GType wintc_sh_view_fs_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_fs_new(
    const gchar* path
);

#endif
