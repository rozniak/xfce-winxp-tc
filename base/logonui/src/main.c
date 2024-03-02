#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "window.h"

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    GtkWidget* window;

    gtk_init(&argc, &argv);

    // Set up theme
    // FIXME: There should probably be an API for this rather than directly
    //        plopping the name straight in GTK
    // FIXME: This should be configurable via INI option and simply default to
    //        Luna for clients, and Classic for servers
    //
    g_object_set(
        gtk_settings_get_default(),
        "gtk-theme-name",
        "Windows XP style (Blue)",
        NULL
    );

    // Create the window and launch
    //
    window = wintc_logonui_window_new();

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
