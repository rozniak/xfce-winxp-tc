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
