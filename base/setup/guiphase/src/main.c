#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "setupwnd.h"

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
    int argc,
    char* argv[]
)
{
    gtk_init(&argc, &argv);

    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        return 0;
    }

    // Set up styling
    //
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/wsetupx/setupwnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Create setup background window
    //
    GtkWidget* wnd_setup = wintc_setup_window_new();

    // Create a billy basic test window (will be replaced by a wizard
    // eventually
    //
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "Hello World!");
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(wnd_setup));

    g_signal_connect(
        window,
        "destroy",
        G_CALLBACK(on_window_destroyed),
        NULL
    );

    // Launch!
    //
    gtk_widget_show_all(wnd_setup);
    gtk_widget_show_all(window);
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
