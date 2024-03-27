#include <glib.h>
#include <wintc/comgtk.h>

#include "fs.h"

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_fs_get_names_as_list(
    const gchar* path,
    gboolean     full_names,
    GError**     error
)
{
    GDir*        dir;
    const gchar* dir_entry = NULL;
    GSList*      entries   = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    WINTC_LOG_DEBUG("fs: enum dir %s", path);

    dir = g_dir_open(path, 0, error);

    if (!dir)
    {
        return NULL;
    }

    while ((dir_entry = g_dir_read_name(dir)))
    {
        gchar* next_name = full_names ?
                               g_build_path(
                                   G_DIR_SEPARATOR_S,
                                   path,
                                   dir_entry,
                                   NULL
                               ) :
                               g_strdup(dir_entry);

        entries = g_slist_append(entries, next_name);
    }

    g_dir_close(dir);

    return entries;
}
