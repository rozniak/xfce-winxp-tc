#include <gio/gunixinputstream.h>
#include <glib.h>
#include <stdlib.h>
#include <wintc/comgtk.h>

//
// STATIC DATA
//
static gchar* S_PKG_CMD_APT[] = {
    "/usr/bin/apt-get",
    "install",
    "-y",
    "-o",
    "APT::Status-Fd=1",
    NULL
};

//
// PUBLIC FUNCTIONS
//
gint wintc_setupapi_exec_install(
    gchar** packages
)
{
    GError* error = NULL;

    // Set up command line
    //
    guint len_cmd  = g_strv_length(S_PKG_CMD_APT);
    guint len_pkgs = g_strv_length(packages);

    gchar** argv = g_malloc0(sizeof(gchar*) * (len_cmd + len_pkgs + 1));

    memcpy(argv, S_PKG_CMD_APT, sizeof(gchar*) * len_cmd);
    memcpy(argv + len_cmd, packages, sizeof(gchar*) * len_pkgs);

    // Spawn APT process
    //
    gint fd;
    GPid pid;

    gboolean success =
        g_spawn_async_with_pipes(
            NULL,
            argv,
            NULL,
            G_SPAWN_DO_NOT_REAP_CHILD,
            NULL,
            NULL,
            &pid,
            NULL,
            &fd,
            NULL,
            &error
        );

    g_free(argv);

    if (!success)
    {
        g_print(
            "ERR %s\n",
            error->message
        );

        g_clear_error(&error);

        return EXIT_FAILURE;
    }

    // Kick off read
    //
    GInputStream*     fd_stream = g_unix_input_stream_new(fd, TRUE);
    gchar*            line;
    int               status    = EXIT_SUCCESS;
    GDataInputStream* stream    = g_data_input_stream_new(fd_stream);

    g_object_unref(fd_stream);

    while (
        line =
            g_data_input_stream_read_line(
                stream,
                NULL,
                NULL,
                &error
            )
    )
    {
        gchar** apt_status = g_strsplit(line, ":", -1);

        if (g_strcmp0(apt_status[0], "pmstatus") == 0) // pmstatus
        {
            g_print(
                "STAT %s %f\n",
                apt_status[1],
                strtod(apt_status[2], NULL)
            );
        }

        g_strfreev(apt_status);
        g_free(line);
    }

    if (error)
    {
        status = EXIT_FAILURE;

        g_print(
            "ERR %s\n",
            error->message
        );

        g_clear_error(&error);
    }
    else
    {
        g_print("%s\n", "STAT done 100.0");
    }

    g_input_stream_close(G_INPUT_STREAM(stream), NULL, NULL);
    g_object_unref(stream);

    g_spawn_close_pid(pid);

    return status;
}
