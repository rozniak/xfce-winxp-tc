#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    WINTC_LOG_DEBUG("WinTC Session Manager starting...");

    // Initial stuff we need to run
    //
    GError* error = NULL;

    gtk_init(&argc, &argv);

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

    // Test running a program 'til exit
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
