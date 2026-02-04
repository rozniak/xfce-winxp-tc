#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "oobewnd.h"

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
    // Spawn GTK
    //
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return EXIT_FAILURE;
    }

    wintc_ctl_install_default_styles();

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
