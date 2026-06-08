#include <glib.h>
#include <gtk/gtk.h>

#include "../public/widget.h"

//
// PUBLIC FUNCTIONS
//
void wintc_widget_set_size_request_natural(
    GtkWidget* widget
)
{
    gint height = 0;
    gint width  = 0;

    gtk_widget_get_preferred_height(widget, NULL, &height);
    gtk_widget_get_preferred_width(widget, NULL, &width);

    gtk_widget_set_size_request(
        widget,
        width,
        height
    );
}
