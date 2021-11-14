#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "window.h"

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
