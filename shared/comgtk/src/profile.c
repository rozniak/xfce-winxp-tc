#include <errno.h>
#include <glib.h>

#include "../public/profile.h"
#include "../public/shorthand.h"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_profile_ensure_exists(
    const gchar* component,
    GError**     out_error
)
{
    int    result;
    gchar* target = wintc_profile_get_path(component, NULL);

    WINTC_SAFE_REF_CLEAR(out_error);

    result = g_mkdir_with_parents(target, 0755);

    g_free(target);

    if (result < 0)
    {
        g_set_error(
            out_error,
            G_FILE_ERROR,
            g_file_error_from_errno(errno),
            "%s",
            g_strerror(errno)
        );

        return FALSE;
    }

    return TRUE;
}

gchar* wintc_profile_get_path(
    const gchar* component,
    const gchar* filename
)
{
    if (filename != NULL)
    {
        return g_build_path(
            "/",
            g_get_user_config_dir(),
            "wintc",
            component,
            filename,
            NULL
        );
    }
    else
    {
        return g_build_path(
            "/",
            g_get_user_config_dir(),
            "wintc",
            component,
            NULL
        );
    }
}

gboolean wintc_profile_get_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar**      contents,
    gsize*       length,
    GError**     error
)
{
    gboolean success;
    gchar*   target = wintc_profile_get_path(component, filename);

    success =
        g_file_get_contents(
            target,
            contents,
            length,
            error
        );
    
    g_free(target);

    return success;
}

gboolean wintc_profile_set_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar*       contents,
    gssize       length,
    GError**     error
)
{
    gboolean success;
    gchar*   target = wintc_profile_get_path(component, filename);

    if (!wintc_profile_ensure_exists(component, error))
    {
        return FALSE;
    }

    success =
        g_file_set_contents(
            target,
            contents,
            length,
            error
        );

    g_free(target);

    return success;
}
