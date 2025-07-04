#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shell.h>

#include "xfsm.h"

//
// FORWARD DECLARATIONS
//
static void smss_complete_startup(void);

static void on_xfsm_host_logout_requested(
    WinTCSmssXfsmHost* host,
    gpointer           user_data
);
static void on_xfsm_host_valid_changed(
    WinTCSmssXfsmHost* host,
    gpointer           user_data
);

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    WINTC_LOG_DEBUG("WinTC Session Manager starting...");

    gtk_init(&argc, &argv);

    // We host org.xfce.SessionManager to implement it ourselves
    //
    WinTCSmssXfsmHost* host = wintc_smss_xfsm_host_new();

    g_signal_connect(
        host,
        "valid-changed",
        G_CALLBACK(on_xfsm_host_valid_changed),
        NULL
    );
    g_signal_connect(
        host,
        "logout-requested",
        G_CALLBACK(on_xfsm_host_logout_requested),
        NULL
    );

    // Begin
    //
    gtk_main();

    return EXIT_SUCCESS;
}

//
// PRIVATE FUNCTIONS
//
static void smss_complete_startup(void)
{
    GError* error = NULL;

    // Spawn the shell
    //
    // FIXME: XFWM4 is obviously an X11 only thing, need to test labwc when
    //        we're started for a Wayland session?
    //
    // FIXME: Session manager should probably take over the power manager role
    //
    const gchar* shell_programs[] = {
        "xfwm4 --compositor=on",
        "wintc-desktop",
        "wintc-taskband",
        "xfce4-power-manager --daemon"
    };

    for (gsize i = 0; i < G_N_ELEMENTS(shell_programs); i++)
    {
        if (
            !wintc_launch_command(
                shell_programs[i],
                &error
            )
        )
        {
            g_critical("%s", "Failed to start shell program.");
            wintc_log_error_and_clear(&error);

            gtk_main_quit();
        }
    }

    wintc_sh_play_sound(WINTC_SHELL_SND_STARTWIN);
}

//
// CALLBACKS
//
static void on_xfsm_host_logout_requested(
    WINTC_UNUSED(WinTCSmssXfsmHost* host),
    WINTC_UNUSED(gpointer user_data)
)
{
    //
    // FIXME: In future this should simply shift the session manager into a
    //        shutdown state, and await applications to close etc.
    //
    gtk_main_quit();
}

static void on_xfsm_host_valid_changed(
    WinTCSmssXfsmHost* host,
    WINTC_UNUSED(gpointer user_data)
)
{
    if (!wintc_smss_xfsm_host_is_valid(host))
    {
        g_critical("%s", "smss: XFSM API host failed to init");
        gtk_main_quit();
    }

    smss_complete_startup();
}
