#ifndef __SHCOMMON_FS_H__
#define __SHCOMMON_FS_H__

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_fs_get_names_as_list(
    const gchar* path,
    gboolean     full_names,
    GFileTest    test,
    gboolean     recursive,
    GError**     error
);

#endif
