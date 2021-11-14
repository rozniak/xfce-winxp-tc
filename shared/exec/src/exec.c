#include <gdk/gdk.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <sys/wait.h>
#include <wintc-comgtk.h>

#include "exec.h"

//
// LOCAL TYPEDEFS
//
typedef gboolean (*CmdParseFunc) (
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
);

//
// FORWARD DECLARATIONS
//
static gchar* expand_desktop_entry_cmdline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
);

static gboolean parse_file_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
);

static gboolean parse_unc_path_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
);

static gboolean parse_url_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
);

static void set_display(
    const gchar* display
);

static gchar** true_shell_parse_argv(
    const gchar* cmdline,
    GError**     out_error
);

//
// FILE SCOPE VARS
//
static CmdParseFunc parsers[] = {
    &parse_unc_path_in_cmdline,
    &parse_url_in_cmdline,
    &parse_file_in_cmdline,
    NULL
};

//
// PUBLIC FUNCTIONS
//
gchar* wintc_desktop_app_info_get_command(
    GDesktopAppInfo* entry
)
{
    GAppInfo* app_info = G_APP_INFO(entry);

    const gchar* cmd_line = g_app_info_get_commandline(app_info);
    const gchar* exe_path = g_app_info_get_executable(app_info);

    if (cmd_line != NULL)
    {
        gchar* expanded;
        gchar* icon_name = g_path_get_basename(exe_path);

        expanded =
            expand_desktop_entry_cmdline(
                cmd_line,
                g_app_info_get_name(app_info),
                icon_name,
                g_desktop_app_info_get_filename(entry),
                FALSE
            );

        g_free(icon_name);

        return expanded;
    }
    else
    {
        return g_strdup(exe_path);
    }
}

gboolean wintc_launch_command(
    const gchar* cmdline,
    GError**     out_error
)
{
    gchar**       argv;
    gchar*        display      = NULL;
    gboolean      done_parsing = FALSE;
    GError*       error        = NULL;
    CmdParseFunc* pparser;
    gchar*        real_cmdline = NULL;
    gchar*        tmp_cmdline  = g_strdup(cmdline);
    gboolean      success;

    WINTC_SAFE_REF_CLEAR(out_error);

    WINTC_LOG_DEBUG("Launching %s", cmdline);

    // Iterate through parsers
    //
    pparser = parsers;

    while (!done_parsing && *pparser != NULL)
    {
        done_parsing =
            (*pparser) (tmp_cmdline, &real_cmdline, &error);

        if (error != NULL)
        {
            g_propagate_error(out_error, error);
            return FALSE;
        }

        WINTC_LOG_DEBUG("Parse result: %s", real_cmdline);

        wintc_strsteal(&tmp_cmdline, &real_cmdline);

        pparser++;
    }

    // Pre-processing complete, set real_cmdline
    //
    real_cmdline = tmp_cmdline;

    WINTC_LOG_DEBUG("Parsed command line: %s", real_cmdline);

    // Parse the command line into an argument vector
    //
    argv = true_shell_parse_argv(real_cmdline, &error);

    g_free(real_cmdline);

    if (argv == NULL)
    {
        g_propagate_error(out_error, error);
        return FALSE;
    }

    // Launch now!
    //
    display =
        g_strdup(
            gdk_display_get_name(gdk_display_get_default())
        );

    success =
        g_spawn_async(
            NULL,
            argv,
            NULL,
            0,
            (GSpawnChildSetupFunc) set_display,
            display,
            NULL,
            &error
        );

    g_free(display);
    g_strfreev(argv);

    if (!success)
    {
        WINTC_LOG_DEBUG("Failed to launch: %s", error->message);

        g_propagate_error(out_error, error);

        return FALSE;
    }

    WINTC_LOG_DEBUG("Done.");

    return TRUE;
}

gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
)
{
    gchar* xdg_query_cmd =
        g_strconcat(
            "xdg-mime query filetype \"",
            filepath,
            "\"",
            NULL
        );

    gchar*   cmd_output = NULL;
    GError*  error      = NULL;
    gint     status;
    gboolean success    = FALSE;

    WINTC_LOG_DEBUG("Querying MIME type for: %s", filepath);

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

        WINTC_LOG_DEBUG("Determined: %s", cmd_output);

        return cmd_output;
    }

    // Handle errors
    //
    if (error != NULL)
    {
        WINTC_LOG_DEBUG("An error occurred: %s", error->message);

        g_propagate_error(out_error, error);

        return NULL;
    }

    WINTC_LOG_DEBUG("Failed with code %d", status);

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

gchar* wintc_query_mime_handler(
    const gchar*      mime_query,
    GError**          out_error,
    GDesktopAppInfo** out_entry
)
{
    gchar* xdg_query_cmd =
        g_strconcat(
            "xdg-mime query default ",
            mime_query,
            NULL
        );

    GDesktopAppInfo* entry      = NULL;
    gchar*           cmd_output = NULL;
    GError*          error      = NULL;
    gchar*           filename   = NULL;
    gboolean         success    = FALSE;

    WINTC_LOG_DEBUG("Querying handler for MIME type %s", mime_query);

    WINTC_SAFE_REF_CLEAR(out_error);
    WINTC_SAFE_REF_CLEAR(out_entry);

    // Run the query
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

    if (success)
    {
        // Did we actually get anything?
        //
        gint output_length = g_utf8_strlen(cmd_output, -1);

        if (output_length == 0)
        {
            WINTC_LOG_DEBUG("No handler found!");

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
        entry    = g_desktop_app_info_new(
                       filename
                   );

        WINTC_LOG_DEBUG("Query returned: %s", filename);

        g_free(cmd_output);
        g_free(filename);
    }
    else
    {
        WINTC_LOG_DEBUG("Failed to query MIME type: %s", error->message);

        g_propagate_error(out_error, error);

        g_free(cmd_output);

        return NULL;
    }

    if (entry == NULL)
    {
        g_set_error(
            out_error,
            G_FILE_ERROR,
            G_FILE_ERROR_FAILED,
            "Unable to load desktop entry for MIME type handler '%s'.",
            mime_query
        );

        WINTC_LOG_DEBUG("Failed to load desktop entry!?");

        return NULL;
    }

    WINTC_SAFE_REF_SET(out_entry, entry);

    return wintc_desktop_app_info_get_command(entry);
}

//
// PRIVATE FUNCTIONS
//
static gchar* expand_desktop_entry_cmdline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
)
{
    GString* expanded = g_string_sized_new(250);

    if (needs_terminal)
    {
        g_string_append(
            expanded,
            "exo-open --launch TerminalEmulator"
        );
    }

    // Iterate through cmdline character by character to expand shortcodes
    //
    const gchar* iter;

    for (iter = cmdline; *iter != '\0'; iter++)
    {
        if (
            iter[0] == '%' &&
            iter[1] != '\0'
        )
        {
            switch (*++iter)
            {
                case 'c':
                    if (name != NULL)
                    {
                        g_string_append(
                            expanded,
                            name
                        );
                    }

                    break;

                case 'i':
                    if (icon_name != NULL)
                    {
                        g_string_append(
                            expanded,
                            name
                        );
                    }

                    break;

                case 'k':
                    g_string_append(
                        expanded,
                        entry_path
                    );
                    break;

                case '%':
                    g_string_append_c(expanded, '%');
                    break;
            }
        }
        else
        {
            g_string_append_c(expanded, *iter);
        }
    }

    return g_string_free(expanded, FALSE);
}

static gboolean parse_file_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
)
{
    GError* error        = NULL;
    gchar*  file_handler;
    gchar*  file_mime;

    WINTC_SAFE_REF_CLEAR(out_cmdline);
    WINTC_SAFE_REF_CLEAR(out_error);

    // See if we can query the MIME type
    //
    file_mime =
        wintc_query_mime_for_file(
            cmdline,
            &error
        );

    if (file_mime == NULL)
    {
        // We don't consider 'file not found' an error here, since we may resolve
        // it later
        //
        if (error->code != G_FILE_ERROR_NOENT)
        {
            g_propagate_error(out_error, error);
        }

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // If it's not an executable, then parse a handler
    //
    if (
        file_mime != NULL &&
        g_strcmp0(file_mime, "application/x-sharedlib") != 0
    )
    {
        WINTC_LOG_DEBUG("Not an executable, will look for handler.");

        file_handler =
            wintc_query_mime_handler(
                file_mime,
                &error,
                NULL
            );

        if (file_handler == NULL)
        {
            WINTC_LOG_DEBUG("I have nothing to handle the file!");

            g_propagate_error(out_error, error);
            WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

            g_free(file_mime);

            return FALSE;
        }

        // We found a handler, build the cmdline now and return
        //
        WINTC_SAFE_REF_SET(
            out_cmdline,
            g_strconcat(
                file_handler,
                " \"",
                cmdline,
                "\"",
                NULL
            )
        );

        g_free(file_handler);
        g_free(file_mime);

        return TRUE;
    }

    // It's an executable, pass on unchanged
    //
    WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

    g_free(file_mime);

    return FALSE;
}

static gboolean parse_unc_path_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
)
{
    static GRegex* unc_regex = NULL;

    gchar*      host;
    GError*     error       = NULL;
    gint        match_count;
    GMatchInfo* match_info;
    gchar*      path;
    gchar*      uri;

    WINTC_SAFE_REF_CLEAR(out_cmdline);
    WINTC_SAFE_REF_CLEAR(out_error);

    // Create regex if it hasn't already been created
    //
    if (unc_regex == NULL)
    {
        unc_regex =
            g_regex_new(
                "^\\\\\\\\([0-9A-Za-z._-]+)(\\\\.+)?",
                0,
                0,
                &error
            );

        if (unc_regex == NULL)
        {
            WINTC_LOG_DEBUG("Failed to create the UNC path regex.");

            g_propagate_error(out_error, error);

            return FALSE;
        }
    }

    // Examine command line
    //
    WINTC_LOG_DEBUG("Checking if %s looks like a UNC path...", cmdline);

    g_regex_match(unc_regex, cmdline, 0, &match_info);

    match_count = g_match_info_get_match_count(match_info);

    // Command line isn't a UNC path, return a duplicate of the original
    //
    if (match_count == 0)
    {
        WINTC_LOG_DEBUG("Nope!");

        g_match_info_free(match_info);

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // Command line IS a UNC path, we need to retrieve the target host and path
    //
    WINTC_LOG_DEBUG("Yeah, looks like a UNC path.");

    host = g_strdup(g_match_info_fetch(match_info, 1));

    if (match_count == 3) // We also have a path!
    {
        path =
            wintc_strsubst(
                g_match_info_fetch(match_info, 2),
                "\\",
                "/"
            );
    }

    g_match_info_free(match_info);

    // Construct URI (doesn't matter if path is NULL, the func stops at the first
    // NULL anyway)
    //
    uri = g_strconcat("smb://", host, path, NULL);

    // Clean up and return
    //
    g_free(host);
    g_free(path);

    WINTC_SAFE_REF_SET(out_cmdline, uri);

    return FALSE;
}

static gboolean parse_url_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
)
{
    static GRegex* url_regex = NULL;

    gchar*      app_name;
    GError*     error         = NULL;
    GString*    final_cmdline;
    GMatchInfo* match_info;
    gchar*      mime_type;
    gchar*      uri_scheme;

    WINTC_SAFE_REF_CLEAR(out_cmdline);
    WINTC_SAFE_REF_CLEAR(out_error);

    // Create regex if it hasn't already been created
    //
    if (url_regex == NULL)
    {
        url_regex =
            g_regex_new(
                "^([A-Za-z-]+)://",
                0,
                0,
                &error
            );

        if (url_regex == NULL)
        {
            WINTC_LOG_DEBUG("Failed to create the URL regex.");

            g_propagate_error(out_error, error);

            return FALSE;
        }
    }

    // Examine command line
    //
    WINTC_LOG_DEBUG("Checking if %s looks like a URL...", cmdline);

    g_regex_match(url_regex, cmdline, 0, &match_info);

    // Command line isn't a URL, return a duplicate of the original
    //
    if (g_match_info_get_match_count(match_info) == 0)
    {
        WINTC_LOG_DEBUG("Nope!");

        g_match_info_free(match_info);

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // Command line IS a URL, retrieve the scheme, query the handling program, return
    // program with URI as argument
    //
    WINTC_LOG_DEBUG("Yeah, looks like a URL.");

    uri_scheme = g_match_info_fetch(match_info, 1);
    mime_type  = g_strconcat(
                     "x-scheme-handler/",
                     uri_scheme,
                     NULL
                 );

    app_name = wintc_query_mime_handler(mime_type, &error, NULL);

    g_match_info_free(match_info);

    if (app_name == NULL)
    {
        WINTC_LOG_DEBUG("I have nothing to handle the URL!");

        g_propagate_error(out_error, error);

        return FALSE;
    }

    final_cmdline = g_string_sized_new(500);

    g_string_append(final_cmdline, app_name);
    g_string_append(final_cmdline, " ");
    g_string_append(final_cmdline, cmdline);

    WINTC_SAFE_REF_SET(out_cmdline, g_string_free(final_cmdline, FALSE));

    return TRUE;
}

static void set_display(
    const gchar* display
)
{
    g_setenv("DISPLAY", display, TRUE);
}

static gchar** true_shell_parse_argv(
    const gchar* cmdline,
    GError**     out_error
)
{
    WINTC_SAFE_REF_CLEAR(out_error);

    gchar** argv;
    GError* error = NULL;

    // Parse cmdline into argv
    //
    g_shell_parse_argv(
        cmdline,
        NULL,
        &argv,
        &error
    );

    if (error != NULL)
    {
        WINTC_LOG_DEBUG("Failed to parse command line: %s", cmdline);

        g_propagate_error(out_error, error);

        return NULL;
    }

    // Resolve path for executable (we might only have the name)
    //
    gchar* resolved_path = g_find_program_in_path(argv[0]);

    if (resolved_path == NULL)
    {
        g_set_error(
            out_error,
            G_FILE_ERROR,
            G_FILE_ERROR_NOENT,
            "Cannot find executable '%s'.",
            argv[0]
        );

        g_strfreev(argv);

        return NULL;
    }

    g_free(argv[0]);
    argv[0] = resolved_path;

    return argv;
}
