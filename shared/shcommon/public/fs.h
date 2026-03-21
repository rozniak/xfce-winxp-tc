#ifndef __SHCOMMON_FS_H__
#define __SHCOMMON_FS_H__

#include <gio/gio.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//
GIcon* wintc_sh_fs_get_file_icon(
    GFile* file
);
GIcon* wintc_sh_fs_get_file_path_icon(
    const gchar* file_path
);

gchar* wintc_sh_fs_get_file_title(
    GFile* file
);
gchar* wintc_sh_fs_get_file_path_title(
    const gchar* file_path
);

GList* wintc_sh_fs_get_names_as_list(
    const gchar* path,
    gboolean     full_names,
    GFileTest    test,
    gboolean     recursive,
    GError**     error
);

#endif
