#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/fs.h"

//
// PUBLIC FUNCTIONS
//
GIcon* wintc_sh_fs_get_file_icon(
    GFile* file
)
{
    GIcon* found_icon;

    // We handle .desktop explicitly
    //
    if (g_str_has_suffix(g_file_peek_path(file), ".desktop"))
    {
        GDesktopAppInfo* app_info =
            g_desktop_app_info_new_from_filename(g_file_peek_path(file));

        found_icon = g_app_info_get_icon(G_APP_INFO(app_info));

        if (found_icon)
        {
            g_object_ref(found_icon);
        }
        else
        {
            found_icon = g_themed_icon_new("empty");
        }

        g_object_unref(app_info);

        return found_icon;
    }

    //
    //
    GFileInfo* file_info =
        g_file_query_info(
            file,
            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
            G_FILE_QUERY_INFO_NONE,
            NULL,
            NULL
        );

    if (!file_info)
    {
        return g_themed_icon_new("empty");
    }

    // Use Gio to look up the icon...
    //
    const gchar* content_type = g_file_info_get_content_type(file_info);

    found_icon =
        content_type ?
            g_content_type_get_icon(content_type) :
            g_themed_icon_new("empty");

    g_object_unref(file_info);

    return found_icon;
}

GIcon* wintc_sh_fs_get_file_path_icon(
    const gchar* file_path
)
{
    GFile* file = g_file_new_for_path(file_path);
    GIcon* ret  = wintc_sh_fs_get_file_icon(file);

    g_object_unref(file);

    return ret;
}

GList* wintc_sh_fs_get_names_as_list(
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
    GList*       entries      = NULL;
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
            g_list_free_full(entries,       g_free);
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
                    g_list_append(
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
