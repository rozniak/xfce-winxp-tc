#include <gdk/gdk.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <sys/wait.h>
#include <wintc/comgtk.h>

#include "../public/desktop.h"
#include "../public/mime.h"
#include "../public/errors.h"
#include "../public/exec.h"

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
static gboolean do_command(
    const gchar* cmdline,
    gboolean     async,
    gchar**      standard_output,
    gchar**      standard_error,
    GError**     out_error
);

static gchar** parse_cmdline(
    const gchar* cmdline,
    GError**     out_error
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
gboolean wintc_launch_command(
    const gchar* cmdline,
    GError**     out_error
)
{
    return do_command(
        cmdline,
        TRUE,
        NULL,
        NULL,
        out_error
    );
}

gboolean wintc_launch_command_sync(
    const gchar* cmdline,
    gchar**      standard_output,
    gchar**      standard_error,
    GError**     out_error
)
{
    return do_command(
        cmdline,
        FALSE,
        standard_output,
        standard_error,
        out_error
    );
}

gboolean wintc_launch_command_with_fallbacks(
    GError**     error,
    const gchar* cmdline,
    ...
)
{
    GError* local_error = NULL;

    // Attempt main cmdline
    //
    if (wintc_launch_command(cmdline, &local_error))
    {
        return TRUE;
    }

    // Failed, catch all except file not found
    //
    if (
        wintc_filter_error(
            local_error,
            G_FILE_ERROR,
            G_FILE_ERROR_NOENT,
            error
        )
    )
    {
        return FALSE;
    }

    g_clear_error(&local_error);

    // Failed... continue to try fallbacks one by one until one works
    //
    va_list      ap;
    const gchar* next_cmdline;

    va_start(ap, cmdline);

    next_cmdline = va_arg(ap, gchar*);

    while (next_cmdline)
    {
        if (wintc_launch_command(cmdline, &local_error))
        {
            va_end(ap);
            return TRUE;
        }

        if (
            wintc_filter_error(
                local_error,
                G_FILE_ERROR,
                G_FILE_ERROR_NOENT,
                error
            )
        )
        {
            return FALSE;
        }

        g_clear_error(&local_error);

        next_cmdline = va_arg(ap, gchar*);
    }

    va_end(ap);

    // Failed to find any of the fallbacks
    //
    g_set_error(
        error,
        WINTC_EXEC_ERROR,
        WINTC_EXEC_ERROR_FELLTHRU,
        "The program and its alternatives are not present on the system."
    );

    return FALSE;
}

//
// PRIVATE FUNCTIONS
//
static gboolean do_command(
    const gchar* cmdline,
    gboolean     async,
    gchar**      standard_output,
    gchar**      standard_error,
    GError**     out_error
)
{
    gchar**  argv;
    gchar*   display;
    gboolean success;
    gint     wait_status = 0;

    WINTC_LOG_USER_DEBUG("Launching %s", cmdline);

    argv = parse_cmdline(cmdline, out_error);

    if (!argv)
    {
        return FALSE;
    }

    display =
        g_strdup(
            gdk_display_get_name(gdk_display_get_default())
        );

    if (async)
    {
        success =
            g_spawn_async(
                NULL,
                argv,
                NULL,
                0,
                (GSpawnChildSetupFunc) set_display,
                display,
                NULL,
                out_error
            );
    }
    else
    {
        success =
            g_spawn_sync(
                NULL,
                argv,
                NULL,
                0,
                (GSpawnChildSetupFunc) set_display,
                display,
                standard_output,
                standard_error,
                &wait_status,
                out_error
            );
    }

    g_free(display);
    g_strfreev(argv);

    if (!success)
    {
        WINTC_LOG_USER_DEBUG("Failed to launch.");

        return FALSE;
    }

    if (
        !g_spawn_check_wait_status(
            wait_status,
            out_error
        )
    )
    {
        WINTC_LOG_USER_DEBUG("Process exited abnormally.");

        return FALSE;
    }

    WINTC_LOG_USER_DEBUG("Done.");

    return TRUE;
}

static gchar** parse_cmdline(
    const gchar* cmdline,
    GError**     out_error
)
{
    gchar**       argv;
    gboolean      done_parsing = FALSE;
    GError*       error        = NULL;
    CmdParseFunc* pparser;
    gchar*        real_cmdline = NULL;
    gchar*        tmp_cmdline  = g_strdup(cmdline);

    WINTC_SAFE_REF_CLEAR(out_error);

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

            g_free(real_cmdline);
            g_free(tmp_cmdline);

            return NULL;
        }

        WINTC_LOG_USER_DEBUG("Parse result: %s", real_cmdline);

        wintc_strsteal(&tmp_cmdline, &real_cmdline);

        pparser++;
    }

    // Pre-processing complete, set real_cmdline
    //
    real_cmdline = tmp_cmdline;

    WINTC_LOG_USER_DEBUG("Parsed command line: %s", real_cmdline);

    // Parse the command line into an argument vector
    //
    argv = true_shell_parse_argv(real_cmdline, &error);

    g_free(real_cmdline);

    if (argv == NULL)
    {
        g_propagate_error(out_error, error);
        return NULL;
    }

    return argv;
}

static gboolean parse_file_in_cmdline(
    const gchar* cmdline,
    gchar**      out_cmdline,
    GError**     out_error
)
{
    GError*          error           = NULL;
    gchar*           file_mime;
    gchar*           handler_cmdline;
    GDesktopAppInfo* handler_entry;

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
        else
        {
            g_clear_error(&error);
        }

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // If it's not an executable, then parse a handler
    //
    if (
        file_mime != NULL &&
        g_strcmp0(file_mime, "application/x-executable") != 0
    )
    {
        WINTC_LOG_USER_DEBUG("Not an executable, will look for handler.");

        handler_entry =
            wintc_query_mime_handler(
                file_mime,
                &error
            );

        if (handler_entry == NULL)
        {
            WINTC_LOG_USER_DEBUG("I have nothing to handle the file!");

            g_propagate_error(out_error, error);
            WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

            g_free(file_mime);

            return FALSE;
        }

        // We found a handler, build the cmdline now and return
        //
        handler_cmdline = wintc_desktop_app_info_get_command(handler_entry);

        WINTC_SAFE_REF_SET(
            out_cmdline,
            g_strdup_printf(
                "%s \"%s\"",
                handler_cmdline,
                cmdline
            )
        );

        g_clear_object(&handler_entry);
        g_free(handler_cmdline);
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
    gchar*      path        = NULL;
    gchar*      uri         = NULL;

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
            WINTC_LOG_USER_DEBUG("Failed to create the UNC path regex.");

            g_propagate_error(out_error, error);

            return FALSE;
        }
    }

    // Examine command line
    //
    WINTC_LOG_USER_DEBUG("Checking if %s looks like a UNC path...", cmdline);

    g_regex_match(unc_regex, cmdline, 0, &match_info);

    match_count = g_match_info_get_match_count(match_info);

    // Command line isn't a UNC path, return a duplicate of the original
    //
    if (match_count == 0)
    {
        WINTC_LOG_USER_DEBUG("Nope!");

        g_match_info_free(match_info);

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // Command line IS a UNC path, we need to retrieve the target host and path
    //
    WINTC_LOG_USER_DEBUG("Yeah, looks like a UNC path.");

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

    // Construct URI (doesn't matter if path is NULL, the func stops at the
    // first NULL anyway)
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
    GError*          error     = NULL;
    GDesktopAppInfo* handler_entry;
    gchar*           handler_cmdline;
    GMatchInfo*      match_info;
    gchar*           mime_type;
    const GRegex*    url_regex = wintc_regex_uri_scheme(&error);
    gchar*           uri_scheme;

    WINTC_SAFE_REF_CLEAR(out_cmdline);
    WINTC_SAFE_REF_CLEAR(out_error);

    if (!url_regex)
    {
        g_propagate_error(out_error, error);
        return FALSE;
    }

    // Examine command line
    //
    WINTC_LOG_USER_DEBUG("Checking if %s looks like a URL...", cmdline);

    g_regex_match(url_regex, cmdline, 0, &match_info);

    // Command line isn't a URL, return a duplicate of the original
    //
    if (g_match_info_get_match_count(match_info) == 0)
    {
        WINTC_LOG_USER_DEBUG("Nope!");

        g_match_info_free(match_info);

        WINTC_SAFE_REF_SET(out_cmdline, g_strdup(cmdline));

        return FALSE;
    }

    // Command line IS a URL, retrieve the scheme
    //
    WINTC_LOG_USER_DEBUG("Yeah, looks like a URL.");

    uri_scheme = g_match_info_fetch(match_info, 1);

    g_match_info_free(match_info);

    // Special case: if the scheme is file:// then convert to just the file
    // path
    //
    if (g_ascii_strcasecmp(uri_scheme, "file") == 0)
    {
        WINTC_SAFE_REF_SET(
            out_cmdline,
            g_uri_unescape_string(
                cmdline + strlen("file://"),
                NULL
            )
        );

        g_free(uri_scheme);

        return FALSE;
    }

    // Continue with normal handling - look up a handler for the scheme
    //
    mime_type = g_strdup_printf(
                    "x-scheme-handler/%s",
                    uri_scheme
                );

    handler_entry = wintc_query_mime_handler(mime_type, &error);

    g_free(mime_type);
    g_free(uri_scheme);

    if (handler_entry == NULL)
    {
        WINTC_LOG_USER_DEBUG("I have nothing to handle the URL!");

        g_propagate_error(out_error, error);

        return FALSE;
    }

    // Output the constructed cmdline, the application cmdline with the URL as the
    // argument
    //
    handler_cmdline = wintc_desktop_app_info_get_command(handler_entry);

    WINTC_SAFE_REF_SET(
        out_cmdline,
        g_strdup_printf(
            "%s %s",
            handler_cmdline,
            cmdline
        )
    );

    g_clear_object(&handler_entry);
    g_free(handler_cmdline);

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
        WINTC_LOG_USER_DEBUG("Failed to parse command line: %s", cmdline);

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
