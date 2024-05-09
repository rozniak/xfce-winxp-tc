#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/fs.h"

//
// PUBLIC FUNCTIONS
//
GSList* wintc_sh_fs_get_names_as_list(
    const gchar* path,
    gboolean     full_names,
    GFileTest    test,
    gboolean     recursive,
    GError**     error
)
{
    GDir*        dir;
    const gchar* dir_entry    = NULL;
    GSList*      dirs_to_enum = NULL;
    GSList*      entries      = NULL;
    GSList*      iter         = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    WINTC_LOG_DEBUG(
        "shcommon: enum dir %s%s",
        path,
        recursive ? " (recursive)" : ""
    );

    // Push requested path as the first item
    //
    dirs_to_enum = g_slist_append(dirs_to_enum, g_strdup(path));
    iter         = dirs_to_enum;

    while (iter)
    {
        WINTC_LOG_DEBUG("shcommon: enum iter dir %s", (gchar*) iter->data);

        dir = g_dir_open((gchar*) iter->data, 0, error);

        if (!dir)
        {
            g_slist_free_full(dirs_to_enum, g_free);
            g_slist_free_full(entries,      g_free);
            return NULL;
        }

        while ((dir_entry = g_dir_read_name(dir)))
        {
            gchar* full_path =
                g_build_path(
                    G_DIR_SEPARATOR_S,
                    (gchar*) iter->data,
                    dir_entry,
                    NULL
                );

            // Take a copy into dirs to enum if needed
            //
            if (recursive && g_file_test(full_path, G_FILE_TEST_IS_DIR))
            {
                WINTC_LOG_DEBUG("shcommon: enum iter, add dir %s", dir_entry);

                dirs_to_enum =
                    g_slist_append(dirs_to_enum, g_strdup(full_path));
            }

            // Move into entries list if it's one we're interested in
            //
            if (!test || g_file_test(full_path, test))
            {
                entries =
                    g_slist_append(
                        entries,
                        g_strdup(full_names ? full_path : dir_entry)
                    );
            }

            g_free(full_path);
        }

        g_dir_close(dir);

        // Iter to next dir
        //
        iter = iter->next;
    }

    g_slist_free_full(dirs_to_enum, g_free);

    return entries;
}
