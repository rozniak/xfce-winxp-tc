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

static const gchar* get_display(void);
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
    GError**  error,
    ...
)
{
    va_list      ap;
    GError*      local_error = NULL;
    const gchar* next_cmdline;

    va_start(ap, error);

    next_cmdline = va_arg(ap, gchar*);

    while (next_cmdline)
    {
        if (wintc_launch_command(next_cmdline, &local_error))
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

    WINTC_LOG_DEBUG("exec: launching %s", cmdline);

    argv = parse_cmdline(cmdline, out_error);

    if (!argv)
    {
        return FALSE;
    }

    display = g_strdup(get_display());

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
        return FALSE;
    }

    if (
        !g_spawn_check_wait_status(
            wait_status,
            out_error
        )
    )
    {
        WINTC_LOG_DEBUG("exec: process exited abnormally");
        return FALSE;
    }

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

        WINTC_LOG_DEBUG("exec: parse result: %s", real_cmdline);

        wintc_strsteal(&tmp_cmdline, &real_cmdline);

        pparser++;
    }

    // Pre-processing complete, set real_cmdline
    //
    real_cmdline = tmp_cmdline;

    WINTC_LOG_DEBUG("exec: final cmdline: %s", real_cmdline);

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
    static const gchar* s_executable_mimes[] =
    {
        "application/x-executable",
        "application/x-shellscript",
        "application/vnd.appimage"
    };

    GError*          error           = NULL;
    gchar*           file_mime       = NULL;
    gchar*           handler_cmdline = NULL;
    GDesktopAppInfo* handler_entry   = NULL;
    gboolean         ret             = FALSE;

    WINTC_LOG_DEBUG("exec: file check");

    // Is the file itself a desktop entry?
    //
    if (g_str_has_suffix(cmdline, ".desktop"))
    {
        handler_entry = g_desktop_app_info_new_from_filename(cmdline);

        if (handler_entry)
        {
            handler_cmdline = wintc_desktop_app_info_get_command(handler_entry);

            *out_cmdline = g_strdup(handler_cmdline);

            ret = TRUE;
        }
        else
        {
            // Do not try and execute this, it's no good
            //
            g_set_error(
                out_error,
                wintc_exec_error_quark(),
                WINTC_EXEC_ERROR_BAD_DESKTOP_ENTRY,
                "The desktop entry could not be parsed."
            );
        }

        goto cleanup;
    }

    // See if we can query the MIME type
    //
    file_mime =
        wintc_query_mime_for_file(
            cmdline,
            &error
        );

    if (file_mime)
    {
        // Check if this is an executable
        //
        for (gsize i = 0; i < G_N_ELEMENTS(s_executable_mimes); i++)
        {
            const gchar* check_mime = s_executable_mimes[i];

            if (g_strcmp0(file_mime, check_mime) == 0)
            {
                // It's an executable, pass on unchanged
                //
                *out_cmdline = g_strdup(cmdline);
                goto cleanup;
            }
        }

        // Not an executable, try to find a handler
        //
        handler_entry =
            wintc_query_mime_handler(
                file_mime,
                &error
            );

        if (handler_entry == NULL)
        {
            g_propagate_error(out_error, error);

            ret = FALSE;
            goto cleanup;
        }

        // We found a handler, build the cmdline now and return
        //
        handler_cmdline = wintc_desktop_app_info_get_command(handler_entry);

        *out_cmdline =
            g_strdup_printf(
                "%s \"%s\"",
                handler_cmdline,
                cmdline
            );

        ret = TRUE;
        goto cleanup;
    }
    else
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

        *out_cmdline = g_strdup(cmdline);

        goto cleanup;
    }

cleanup:
    g_clear_object(&handler_entry);
    g_free(handler_cmdline);
    g_free(file_mime);

    return ret;
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

    WINTC_LOG_DEBUG("exec: UNC path check");

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
            g_propagate_error(out_error, error);
            return FALSE;
        }
    }

    // Examine command line
    //
    g_regex_match(unc_regex, cmdline, 0, &match_info);

    match_count = g_match_info_get_match_count(match_info);

    // Command line isn't a UNC path, return a duplicate of the original
    //
    if (match_count == 0)
    {
        g_match_info_free(match_info);

        *out_cmdline = g_strdup(cmdline);

        return FALSE;
    }

    // Command line IS a UNC path, we need to retrieve the target host and path
    //
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

    // Construct URI
    //
    uri = g_strconcat("smb://", host, path, NULL);

    *out_cmdline = uri;

    g_free(host);
    g_free(path);

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

    WINTC_LOG_DEBUG("exec: URL check");

    if (!url_regex)
    {
        g_propagate_error(out_error, error);
        return FALSE;
    }

    // Examine command line
    //
    g_regex_match(url_regex, cmdline, 0, &match_info);

    // Command line isn't a URL, return a duplicate of the original
    //
    if (g_match_info_get_match_count(match_info) == 0)
    {
        g_match_info_free(match_info);

        *out_cmdline = strdup(cmdline);

        return FALSE;
    }

    // Command line IS a URL, retrieve the scheme
    //
    uri_scheme = g_match_info_fetch(match_info, 1);

    g_match_info_free(match_info);

    // Special case: if the scheme is file:// then convert to just the file
    // path
    //
    if (g_ascii_strcasecmp(uri_scheme, "file") == 0)
    {
        *out_cmdline =
            g_uri_unescape_string(
                cmdline + strlen("file://"),
                NULL
            );

        g_free(uri_scheme);

        return FALSE;
    }

    // Continue with normal handling - look up a handler for the scheme
    //
    mime_type =
        g_strdup_printf(
            "x-scheme-handler/%s",
            uri_scheme
        );

    handler_entry = wintc_query_mime_handler(mime_type, &error);

    g_free(mime_type);
    g_free(uri_scheme);

    if (handler_entry == NULL)
    {
        g_propagate_error(out_error, error);
        return FALSE;
    }

    // Output the constructed cmdline, the application cmdline with the URL as the
    // argument
    //
    handler_cmdline = wintc_desktop_app_info_get_command(handler_entry);

    *out_cmdline =
        g_strdup_printf(
            "%s %s",
            handler_cmdline,
            cmdline
        );

    g_clear_object(&handler_entry);
    g_free(handler_cmdline);

    return TRUE;
}

static const gchar* get_display(void)
{
    GdkDisplay* display = gdk_display_get_default();

    if (display)
    {
        return gdk_display_get_name(display);
    }

    return NULL;
}

static void set_display(
    const gchar* display
)
{
    if (display)
    {
        g_setenv("DISPLAY", display, TRUE);
    }
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
