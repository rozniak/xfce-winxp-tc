#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "settings.h"
#include "window.h"

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    gtk_init(&argc, &argv);

    WinTCLogonUISettings* settings = wintc_logonui_settings_new();

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
    GtkWidget* window = wintc_logonui_window_new(settings);

    g_object_unref(settings);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
}
