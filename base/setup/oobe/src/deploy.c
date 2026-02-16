#include <glib.h>
#include <sys/stat.h>
#include <wintc/comgtk.h>

#include "deploy.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_oobe_deploy_drop_file(
    const gchar* resource_path,
    const gchar* drop_directory,
    const gchar* drop_filename,
    GError**     error
)
{
    WINTC_LOG_DEBUG(
        "oobe: deploying %s to %s/%s",
        resource_path,
        drop_directory,
        drop_filename
    );

    // Ensure the dir exists
    //
    if (g_mkdir_with_parents(drop_directory, S_IRWXU) < 0)
    {
        g_set_error(
            error,
            g_file_error_quark(),
            g_file_error_from_errno(errno),
            "Failed to create directory: %s (%d)",
            drop_directory,
            errno
        );

        return FALSE;
    }

    // Get the resource
    //
    GBytes*      resource_bytes;
    const gchar* resource_data;

    resource_bytes =
        g_resources_lookup_data(
            resource_path,
            G_RESOURCE_LOOKUP_FLAGS_NONE,
            error
        );

    if (!resource_bytes)
    {
        return FALSE;
    }

    resource_data =
        g_bytes_get_data(resource_bytes, NULL);

    // Drop the file
    //
    gchar* target_path =
        g_build_path(
            G_DIR_SEPARATOR_S,
            drop_directory,
            drop_filename,
            NULL
        );

    gboolean success =
        g_file_set_contents(
            target_path,
            resource_data,
            -1,
            error
        );

    g_bytes_unref(resource_bytes);
    g_free(target_path);

    return success;
}
