#include <glib.h>
#include <gio/gunixinputstream.h>
#include <wintc/comgtk.h>

#include "setupapi.h"

//
// PRIVATE STRUCTS
//
typedef struct _WinTCSetupActCallbacks
{
    WinTCSetupActErrorCallback    error_cb;
    WinTCSetupActProgressCallback progress_cb;
    gpointer                      user_data;
} WinTCSetupActCallbacks;

//
// FORWARD DECLARATIONS
//
static void cb_read_line_pkgmgr(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);

//
// PUBLIC CONSTANTS
//
gchar* WINTC_SETUP_ACT_PKG_PATH = NULL;

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

static GList* S_INSTALLED_PACKAGES = NULL;

//
// PUBLIC FUNCTIONS
//
gboolean wintc_setup_act_init(void)
{
    GError* error = NULL;

    // The package path is logged out in <setuproot>/pkgpath
    //
    gchar* packages_path_descr =
        g_build_path(
            G_DIR_SEPARATOR_S,
            WINTC_SETUP_ACT_ROOT_DIR,
            "pkgpath",
            NULL
        );

    if (
        !g_file_get_contents(
            packages_path_descr,
            &WINTC_SETUP_ACT_PKG_PATH,
            NULL,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    g_free(packages_path_descr);

    return !!(WINTC_SETUP_ACT_PKG_PATH);
}

gboolean wintc_setup_act_install_packages(
    GList*                        list_packages,
    WinTCSetupActErrorCallback    error_callback,
    WinTCSetupActProgressCallback progress_callback,
    gpointer                      user_data,
    GError**                      error
)
{
    //
    // FIXME: This is ONLY for apt right now, other package managers coming
    //        soon!
    //

    // Create the command and launch the package manager
    //
    gint fd_out = 1;

    guint len_cmd      = g_strv_length(S_PKG_CMD_APT);
    guint len_packages = g_list_length(list_packages);

    gchar** argv = g_malloc0(sizeof(gchar*) * (len_cmd + len_packages + 1));

    gint i = len_cmd;

    memcpy(argv, S_PKG_CMD_APT, len_packages);

    for (GList* iter = list_packages; iter; iter = iter->next, i++)
    {
        argv[i] = iter->data;
    }

    gboolean success =
        g_spawn_async_with_pipes(
            NULL,
            argv,
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            NULL,
            &fd_out,
            NULL,
            error
        );

    g_free(argv);

    if (!success)
    {
        return FALSE;
    }

    // Build our callback struct
    //
    WinTCSetupActCallbacks* callbacks =
        g_new0(WinTCSetupActCallbacks, 1);

    callbacks->error_cb    = error_callback;
    callbacks->progress_cb = progress_callback;
    callbacks->user_data   = user_data;

    // Kick off read
    //
    GInputStream*     fd_stream = g_unix_input_stream_new(fd_out, FALSE);
    GDataInputStream* stream    = g_data_input_stream_new(fd_stream);

    g_data_input_stream_read_line_async(
        stream,
        G_PRIORITY_DEFAULT,
        NULL,
        cb_read_line_pkgmgr,
        callbacks
    );

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void cb_read_line_pkgmgr(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GDataInputStream* stream = G_DATA_INPUT_STREAM(source_object);

    WinTCSetupActCallbacks* callbacks =
        (WinTCSetupActCallbacks*) user_data;

    GError* error = NULL;

    gchar* line =
        g_data_input_stream_read_line_finish(
            stream,
            res,
            NULL,
            &error
        );

    if (!line)
    {
        if (error)
        {
            callbacks->error_cb(
                &error,
                callbacks->user_data
            );
            return;
        }

        // Write out to disk
        //
        gchar* installed_packages =
            wintc_list_implode_strings(S_INSTALLED_PACKAGES);

        if (
            !g_file_set_contents(
                WINTC_SETUP_ACT_ROOT_DIR,
                installed_packages,
                -1,
                &error
            )
        )
        {
            wintc_log_error_and_clear(&error);
        }

        g_free(installed_packages);

        // We're done!
        //
        callbacks->progress_cb(
            100.0f,
            callbacks->user_data
        );

        return;
    }

    // Deal with APT
    //
    gchar** apt_status = g_strsplit(line, ":", -1);

    if (g_str_has_suffix(apt_status[0], "status")) // dlstatus / pmstatus
    {
        // Update progress
        //
        callbacks->progress_cb(
            strtod(apt_status[2], NULL),
            callbacks->user_data
        );

        // For pmstatus, track the packages we're installing
        //
        if (
            g_strcmp0(apt_status[0], "pmstatus") == 0 &&
            g_str_has_prefix(apt_status[3], "Installed")
        )
        {
            S_INSTALLED_PACKAGES =
                g_list_prepend(S_INSTALLED_PACKAGES, apt_status[1]);
        }
    }

    g_strfreev(apt_status);
    g_free(line);

    // Wait for next line
    //
    g_data_input_stream_read_line_async(
        stream,
        G_PRIORITY_DEFAULT,
        NULL,
        cb_read_line_pkgmgr,
        callbacks
    );
}
