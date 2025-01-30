#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "../public/window.h"

//
// PUBLIC FUNCTIONS
//
void wintc_focus_window(
    GtkWindow* window
)
{
    gtk_window_present_with_time(
        window,
        GDK_CURRENT_TIME
    );
}

GtkWindow* wintc_widget_get_toplevel_window(
    GtkWidget* widget
)
{
    GtkWidget* toplevel = gtk_widget_get_toplevel(widget);

    if (GTK_IS_WINDOW(toplevel))
    {
        return GTK_WINDOW(toplevel);
    }

    return NULL;
}
