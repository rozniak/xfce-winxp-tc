#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <sys/wait.h>
#include <wintc/comgtk.h>

#include "../public/mime.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
)
{
    gchar* xdg_query_cmd =
        g_strdup_printf(
            "xdg-mime query filetype \"%s\"",
            filepath
        );

    gchar*   cmd_output = NULL;
    GError*  error      = NULL;
    gint     status;
    gboolean success    = FALSE;

    WINTC_LOG_USER_DEBUG("Querying MIME type for: %s", filepath);

    WINTC_SAFE_REF_CLEAR(out_error);

    // Run the query
    //
    success =
        g_spawn_command_line_sync(
            xdg_query_cmd,
            &cmd_output,
            NULL,
            &status,
            &error
        );

    status = WEXITSTATUS(status);

    g_free(xdg_query_cmd);

    if (success && status == 0)
    {
        g_strstrip(cmd_output);

        WINTC_LOG_USER_DEBUG("Determined: %s", cmd_output);

        return cmd_output;
    }

    g_free(cmd_output);

    // Handle errors
    //
    if (error != NULL)
    {
        WINTC_LOG_USER_DEBUG("An error occurred: %s", error->message);

        g_propagate_error(out_error, error);

        return NULL;
    }

    WINTC_LOG_USER_DEBUG("Failed with code %d", status);

    switch (status)
    {
        case 2: // File not found
            g_set_error(
                out_error,
                G_FILE_ERROR,
                G_FILE_ERROR_NOENT,
                "Cannot find file or folder '%s'.",
                filepath
            );

            break;

        default:
            g_set_error(
                out_error,
                G_FILE_ERROR,
                G_FILE_ERROR_FAILED,
                "Unknown error occurred."
            );

            break;
    }

    return NULL;
}

GDesktopAppInfo* wintc_query_mime_handler(
    const gchar* mime_query,
    GError**     out_error
)
{
    gchar* xdg_query_cmd =
        g_strdup_printf(
            "xdg-mime query default %s",
            mime_query
        );

    gchar*           cmd_output = NULL;
    GDesktopAppInfo* entry      = NULL;
    GError*          error      = NULL;
    gchar*           filename   = NULL;
    gboolean         success    = FALSE;

    WINTC_LOG_USER_DEBUG("Querying handler for MIME type %s", mime_query);

    WINTC_SAFE_REF_CLEAR(out_error);

    // Run query
    //
    success =
        g_spawn_command_line_sync(
            xdg_query_cmd,
            &cmd_output,
            NULL,
            NULL,
            &error
        );

    g_free(xdg_query_cmd);

    if (!success)
    {
        WINTC_LOG_USER_DEBUG("Failed to query MIME type: %s", error->message);

        g_propagate_error(out_error, error);

        g_free(cmd_output);

        return NULL;
    }

    // Did we really get anything
    //
    gint output_length = g_utf8_strlen(cmd_output, -1);

    if (output_length == 0)
    {
        WINTC_LOG_USER_DEBUG("No handler found!");

        g_set_error(
            out_error,
            G_FILE_ERROR,
            G_FILE_ERROR_NOSYS,
            "No program is mapped to MIME type '%s'.",
            mime_query
        );

        g_free(cmd_output);

        return NULL;
    }

    // We did! It's a desktop entry, so retrieve it
    //
    filename = g_utf8_substring(
                   cmd_output,
                   0,
                   g_utf8_strlen(cmd_output, -1) - 1
               );
    entry    = g_desktop_app_info_new(filename);

    WINTC_LOG_USER_DEBUG("Query returned: %s", filename);

    g_free(cmd_output);
    g_free(filename);

    if (entry == NULL)
    {
        g_set_error(
            out_error,
            G_FILE_ERROR,
            G_FILE_ERROR_FAILED,
            "Unable to load desktop entry for MIME type handler '%s'.",
            mime_query
        );

        WINTC_LOG_USER_DEBUG("Association found, but the desktop entry doesn't exist.");

        return NULL;
    }

    return entry;
}
