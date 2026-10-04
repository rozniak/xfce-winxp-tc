#include <glib.h>
#include <gio/gunixinputstream.h>
#include <wintc/comgtk.h>
#include <wintc/setupapi.h>

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
static gboolean wintc_setup_act_spawn_ping(
    WinTCSetupActCallbacks* callbacks,
    GError**                error
);

static gboolean cb_timeout_retry_ping(
    gpointer user_data
);
static void cb_watch_ping(
    GPid     pid,
    gint     status,
    gpointer user_data
);
static gboolean cb_idle_next_setting_phase(
    gpointer user_data
);

static void on_pkg_session_done(
    WinTCPkgSession* session,
    gpointer         user_data
);
static void on_pkg_session_progress(
    WinTCPkgSession* session,
    gdouble          progress,
    gpointer         user_data
);

//
// PUBLIC CONSTANTS
//
gchar* WINTC_SETUP_ACT_PKG_PATH = NULL;

//
// STATIC DATA
//
static gint S_SETTING_PHASE = -1;

static WinTCPkgSession* S_PKG_SESSION = NULL;

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
    // Build our callback struct
    //
    WinTCSetupActCallbacks* callbacks =
        g_new0(WinTCSetupActCallbacks, 1);

    callbacks->done_cb     = done_callback;
    callbacks->error_cb    = error_callback;
    callbacks->progress_cb = progress_callback;
    callbacks->user_data   = user_data;

    // Build packaging session
    //
    g_clear_object(&S_PKG_SESSION);

    S_PKG_SESSION = wintc_pkg_session_new(list_packages);

    g_signal_connect(
        S_PKG_SESSION,
        "done",
        G_CALLBACK(on_pkg_session_done),
        callbacks
    );
    g_signal_connect(
        S_PKG_SESSION,
        "progress",
        G_CALLBACK(on_pkg_session_progress),
        callbacks
    );

    // HACK: On Arch Linux we may be too quick during start up and the network
    //       isn't ready -- do a ping to determine we have a connection before
    //       starting the packaging session
    //
    if (!wintc_setup_act_spawn_ping(callbacks, error))
    {
        return FALSE;
    }

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
    S_SETTING_PHASE = -1;

    g_idle_add(
        (GSourceFunc) cb_idle_next_setting_phase,
        callbacks
    );
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
                goto cleanup_lightdm;
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
            }

cleanup_lightdm:
            g_free(key_raw);
            g_key_file_unref(key_file);

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

static gboolean wintc_setup_act_spawn_ping(
    WinTCSetupActCallbacks* callbacks,
    GError**                error
)
{
    static gchar* s_ping_argv[] = {
        WINTC_RT_PREFIX "/bin/ping",
        "-c",
        "1",
        "-w",
        "1",
        "8.8.8.8",
        NULL
    };

    GPid pid;

    if (
        !g_spawn_async(
            NULL,
            s_ping_argv,
            NULL,
            G_SPAWN_DO_NOT_REAP_CHILD,
            NULL,
            NULL,
            &pid,
            error
        )
    )
    {
        return FALSE;
    }

    // Watch the ping
    //
    g_child_watch_add(
        pid,
        (GChildWatchFunc) cb_watch_ping,
        callbacks
    );

    return TRUE;
}

//
// CALLBACKS
//
static gboolean cb_timeout_retry_ping(
    gpointer user_data
)
{
    WinTCSetupActCallbacks* callbacks = 
        (WinTCSetupActCallbacks*) user_data;

    GError* error = NULL;

    if (!wintc_setup_act_spawn_ping(callbacks, &error))
    {
        wintc_setup_act_raise_error(callbacks, &error);
    }

    return G_SOURCE_REMOVE;
}

static void cb_watch_ping(
    WINTC_UNUSED(GPid pid),
    gint     status,
    gpointer user_data
)
{
    static gint s_attempts = 0;

    WinTCSetupActCallbacks* callbacks = 
        (WinTCSetupActCallbacks*) user_data;

    GError* error = NULL;

    if (g_spawn_check_wait_status(status, &error))
    {
        // Kick off package session
        //
        wintc_pkg_session_begin(S_PKG_SESSION);
    }
    else
    {
        if (s_attempts++ < 3)
        {
            g_timeout_add_seconds(
                5,
                (GSourceFunc) cb_timeout_retry_ping,
                callbacks
            );
        }
        else
        {
            wintc_setup_act_raise_error(callbacks, &error);
        }
    }
}
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

static void on_pkg_session_done(
    WinTCPkgSession* session,
    gpointer         user_data
)
{
    WinTCSetupActCallbacks* callbacks =
        (WinTCSetupActCallbacks*) user_data;

    GError* error = NULL;

    if (!wintc_pkg_session_get_successful(session, &error))
    {
        wintc_setup_act_raise_error(callbacks, &error);
        return;
    }

    callbacks->done_cb(
        callbacks->user_data
    );
}

static void on_pkg_session_progress(
    WINTC_UNUSED(WinTCPkgSession* session),
    gdouble  progress,
    gpointer user_data
)
{
    WinTCSetupActCallbacks* callbacks =
        (WinTCSetupActCallbacks*) user_data;

    callbacks->progress_cb(
        progress,
        callbacks->user_data
    );
}
