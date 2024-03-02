#include <glib.h>
#include <stddef.h>
#include <wintc/comgtk.h>

#include "history.h"

#define HISTORY_FILENAME "run-history"
#define HISTORY_MAX_SIZE 26

//
// LOCAL RESOURCES
//
static GList* internal_history_list = NULL;

//
// PUBLIC FUNCTIONS
//
GList* wintc_get_run_history(
    GError** out_error
)
{
    GError*  error         = NULL;
    gchar*   history_text  = NULL;
    gboolean read_success;

    WINTC_SAFE_REF_CLEAR(out_error);

    // Attempt to retrieve file
    //
    read_success =
        wintc_profile_get_file_contents(
            WINTC_COMPONENT_SHELL,
            HISTORY_FILENAME,
            &history_text,
            NULL,
            &error
        );

    if (!read_success)
    {
        WINTC_LOG_USER_DEBUG("No run history: %s", error->message);

        g_propagate_error(out_error, error);

        return NULL;
    }

    // Parse into a list
    //
    internal_history_list =
        wintc_list_read_from_string(history_text);

    g_free(history_text);

    return internal_history_list;
}

gboolean wintc_append_run_history(
    const gchar* cmdline,
    GError**     out_error
)
{
    GError*  error        = NULL;
    gchar*   history_file;
    gboolean success;

    WINTC_SAFE_REF_CLEAR(out_error);

    WINTC_LOG_USER_DEBUG("Writing to run history");

    // Check if list is above maximum size?
    //
    if (internal_history_list != NULL)
    {
        internal_history_list =
            wintc_list_limit(
                internal_history_list,
                HISTORY_MAX_SIZE - 1,
                (GDestroyNotify) g_free
            );
    }
    
    // Add latest item to the front
    //
    internal_history_list =
        wintc_list_distinct_prepend(
            internal_history_list,
            g_strdup(cmdline),
            (GCompareFunc) g_strcmp0,
            (GDestroyNotify) g_free
        );

    // Iterate through and create text file
    //
    history_file =
        wintc_list_implode_strings(internal_history_list);

    success =
        wintc_profile_set_file_contents(
            WINTC_COMPONENT_SHELL,
            HISTORY_FILENAME,
            history_file,
            -1,
            &error
        );

    g_free(history_file);

    if (!success)
    {
        WINTC_LOG_USER_DEBUG("Failed to write run history: %s", error->message);

        g_propagate_error(out_error, error);

        return FALSE;
    }

    return TRUE;
}
