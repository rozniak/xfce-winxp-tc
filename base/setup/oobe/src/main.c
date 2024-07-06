#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "window.h"

//
// FORWARD DECLARATIONS
//
static void on_window_destroyed(
    GtkWidget* widget,
    gpointer   user_data
);

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return 1;
    }

    // Create OOBE window
    //
    GtkWidget* wnd_oobe = wintc_oobe_window_new();

    g_signal_connect(
        wnd_oobe,
        "destroy",
        G_CALLBACK(on_window_destroyed),
        NULL
    );

    gtk_widget_show_all(wnd_oobe);

    gtk_main();

    return 0;
}

//
// CALLBACKS
//
static void on_window_destroyed(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gpointer   user_data)
)
{
    gtk_main_quit();
}
