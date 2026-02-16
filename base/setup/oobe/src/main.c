#include <errno.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <sys/stat.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shelldpa.h>

#include "oobewnd.h"

#define WINTC_SETUP_ROOT_DIR "/var/tmp/.wintc-setup"

//
// FORWARD DECLARATIONS
//
static void on_wnd_oobe_destroy(
    GtkWidget* object,
    gpointer   user_data
);

//
// ENTRY POINT
//
int main(
    int argc,
    char* argv[]
)
{
    GError* error = NULL;

    // Before all else, write to the phase file so if we crash, the OOBE
    // doesn't start again
    //
    if (g_mkdir_with_parents(WINTC_SETUP_ROOT_DIR, S_IRWXU) < 0)
    {
        g_critical(
            "oobe: failed to create %s (err: %d)",
            WINTC_SETUP_ROOT_DIR,
            errno
        );
        return EXIT_FAILURE;
    }

    if (
        !g_file_set_contents(
            WINTC_SETUP_ROOT_DIR "/phase",
            "3",
            -1,
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

    // Spawn GTK
    //
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return EXIT_FAILURE;
    }

    wintc_ctl_install_default_styles();

    // Fire up XFWM4
    //
    if (!wintc_launch_command("xfwm4 --compositor=on", &error))
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

    // Spawn GStreamer
    //
    gst_init(&argc, &argv);

    // Set GtkSettings, because xsettings never seems to work -_-
    //
    GtkSettings* settings = gtk_settings_get_default();

    g_object_set(
        settings,
        "gtk-theme-name",        "Windows Classic style",
        "gtk-cursor-theme-name", "standard-with-shadow",
        NULL
    );

    // Create setup background window
    //
    GtkWidget* wnd_oobe = wintc_oobe_window_new();

    g_signal_connect(
        wnd_oobe,
        "destroy",
        G_CALLBACK(on_wnd_oobe_destroy),
        NULL
    );

    // Launch!
    //
    gtk_widget_show_all(wnd_oobe);
    gtk_main();

    return EXIT_SUCCESS;
}

//
// CALLBACKS
//
static void on_wnd_oobe_destroy(
    WINTC_UNUSED(GtkWidget* object),
    WINTC_UNUSED(gpointer   user_data)
)
{
    gtk_main_quit();
}
