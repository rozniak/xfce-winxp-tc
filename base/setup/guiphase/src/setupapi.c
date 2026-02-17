#include <glib.h>
#include <gio/gunixinputstream.h>
#include <wintc/comgtk.h>

#include "setupapi.h"

//
// PRIVATE ENUMS
//
enum
{
    WINTC_SETUP_ACT_PHASE_DEPLOY_LIGHTDM_CONF,
    WINTC_SETUP_ACT_PHASE_DONE,

    N_SETTINGS_PHASES
};

//
// PRIVATE STRUCTS
//
typedef struct _WinTCSetupActCallbacks
{
    WinTCSetupActDoneCallback     done_cb;
    WinTCSetupActErrorCallback    error_cb;
    WinTCSetupActProgressCallback progress_cb;
    gpointer                      user_data;
} WinTCSetupActCallbacks;

//
// FORWARD DECLARATIONS
//
static void wintc_setup_act_iter_setting_phase(
    WinTCSetupActCallbacks* callbacks
);
static void wintc_setup_act_raise_error(
    WinTCSetupActCallbacks* callbacks,
    GError**                error
);

static gboolean cb_idle_next_setting_phase(
    gpointer user_data
);
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
static gint   S_SETTING_PHASE      = 0;

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

    if (WINTC_SETUP_ACT_PKG_PATH)
    {
        g_strstrip(WINTC_SETUP_ACT_PKG_PATH);
    }

    return !!(WINTC_SETUP_ACT_PKG_PATH);
}

gboolean wintc_setup_act_install_packages(
    GList*                        list_packages,
    WinTCSetupActDoneCallback     done_callback,
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

    memcpy(argv, S_PKG_CMD_APT, sizeof(gchar*) * len_cmd);

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

    callbacks->done_cb     = done_callback;
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

void wintc_setup_act_prepare_system(
    WinTCSetupActDoneCallback     done_callback,
    WinTCSetupActErrorCallback    error_callback,
    WinTCSetupActProgressCallback progress_callback,
    gpointer                      user_data
)
{
    // Build our callback struct
    //
    WinTCSetupActCallbacks* callbacks =
        g_new0(WinTCSetupActCallbacks, 1);

    callbacks->done_cb     = done_callback;
    callbacks->error_cb    = error_callback;
    callbacks->progress_cb = progress_callback;
    callbacks->user_data   = user_data;

    // Reset phase
    //
    S_SETTING_PHASE = 0;

    wintc_setup_act_iter_setting_phase(callbacks);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_setup_act_iter_setting_phase(
    WinTCSetupActCallbacks* callbacks
)
{
    gboolean async = FALSE;
    GError*  error = NULL;

    switch (S_SETTING_PHASE)
    {
        case WINTC_SETUP_ACT_PHASE_DEPLOY_LIGHTDM_CONF:
        {
            // FIXME: Might need update-alternatives for Ubuntu
            // 
            GKeyFile* key_file = g_key_file_new();
            gchar*    key_raw  = NULL;

            if (
                !g_key_file_load_from_file(
                    key_file,
                    "/etc/lightdm/lightdm.conf",
                    G_KEY_FILE_KEEP_COMMENTS,
                    &error
                )
            )
            {
                wintc_setup_act_raise_error(callbacks, &error);
                return;
            }

            g_key_file_set_string(
                key_file,
                "Seat:*",
                "greeter-session",
                "wintc-logonui"
            );

            key_raw =
                g_key_file_to_data(key_file, NULL, NULL);

            if (
                !g_file_set_contents(
                    "/etc/lightdm/lightdm.conf",
                    key_raw,
                    -1,
                    &error
                )
            )
            {
                wintc_setup_act_raise_error(callbacks, &error);
                return;
            }

            break;
        }

        case WINTC_SETUP_ACT_PHASE_DONE:
            callbacks->done_cb(
                callbacks->user_data
            );

            g_free(callbacks);
            break;
    }

    // Only proceed to the next phase if we're not waiting for something
    // async to finish
    //
    // Also add this on idle, so that the UI can update in between phases
    //
    if (!async)
    {
        g_idle_add(
            (GSourceFunc) cb_idle_next_setting_phase,
            callbacks
        );
    }
}

static void wintc_setup_act_raise_error(
    WinTCSetupActCallbacks* callbacks,
    GError**                error
)
{
    callbacks->error_cb(
        error,
        callbacks->user_data
    );

    g_clear_error(error); // Just in case
    g_free(callbacks);
}

//
// CALLBACKS
//
static gboolean cb_idle_next_setting_phase(
    gpointer user_data
)
{
    WinTCSetupActCallbacks* callbacks = 
        (WinTCSetupActCallbacks*) user_data;

    S_SETTING_PHASE++;

    callbacks->progress_cb(
        (gdouble) S_SETTING_PHASE / N_SETTINGS_PHASES,
        callbacks->user_data
    );

    wintc_setup_act_iter_setting_phase(callbacks);

    return G_SOURCE_REMOVE;
}

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
            wintc_setup_act_raise_error(callbacks, &error);
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
        callbacks->done_cb(
            callbacks->user_data
        );

        g_free(callbacks);

        return;
    }

    // Deal with APT
    //
    gchar** apt_status = g_strsplit(line, ":", -1);

    if (g_strcmp0(apt_status[0], "pmstatus") == 0) // pmstatus
    {
        // Update progress
        //
        callbacks->progress_cb(
            strtod(apt_status[2], NULL),
            callbacks->user_data
        );

        // Track the packages we're installing
        //
        if (g_str_has_prefix(apt_status[3], "Installed"))
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
