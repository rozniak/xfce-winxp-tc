#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shell.h>

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    WINTC_LOG_DEBUG("WinTC Session Manager starting...");

    //
    // Initial stuff we need to run
    //
    GError* error = NULL;

    gtk_init(&argc, &argv);

    // Spawn xfsettingsd - responsible for synchronising xsettings stuff to GTK
    // and whatnot (GTK / sound theme, etc.)
    //
    // FIXME: One day this should be rolled up into regsvc to synchronise to
    //        xsettings (for X11) and GSettings (for Wayland), along with the
    //        xfconf stuff
    //
    if (
        !wintc_launch_command_sync(
            "xfsettingsd --replace --daemon",
            NULL,
            NULL,
            &error
        )
    )
    {
        g_critical("%s", "Failed to launch xfsettingsd.");
        wintc_log_error_and_clear(&error);

        return EXIT_FAILURE;
    }

    // Trigger settings load - we must request settings and wait for them to be
    // loaded in the GTK event loop so that the startup sound can play
    //
    gtk_settings_get_for_screen(gdk_screen_get_default());

    while (gtk_events_pending())
    {
        gtk_main_iteration();
    }

    // Spawn the shell
    //
    // FIXME: XFWM4 is obviously an X11 only thing, need to test labwc when
    //        we're started for a Wayland session?
    //
    const gchar* shell_programs[] = {
        "xfwm4 --compositor=on",
        "wintc-desktop",
        "wintc-taskband"
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

            return EXIT_FAILURE;
        }
    }

    // Play the startup sound now the shell should be spawning
    //
    wintc_sh_play_sound(WINTC_SHELL_SND_STARTWIN);

    // TODO: Replace this with gtk_main() and spawn the actual session
    //       management DBus listeners and stuff
    //
    //       For now we're just running winver synchronously so that the
    //       session can be tested (ie. session ends when winver is closed)
    //
    if (
        !wintc_launch_command_sync(
            "winver",
            NULL,
            NULL,
            &error
        )
    )
    {
        g_critical("%s", "Failed to launch test program.");
        wintc_log_error_and_clear(&error);

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
