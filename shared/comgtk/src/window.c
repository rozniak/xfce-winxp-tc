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

void wintc_window_move_to_center(
    GtkWindow* window
)
{
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(gdk_display_get_default());

    GdkRectangle rect_monitor;
    GdkRectangle rect_widget;

    gtk_widget_get_allocation(GTK_WIDGET(window), &rect_widget);
    gdk_monitor_get_geometry(monitor, &rect_monitor);

    gtk_window_move(
        window,
        rect_monitor.x + (rect_monitor.width  / 2) - (rect_widget.width  / 2),
        rect_monitor.y + (rect_monitor.height / 2) - (rect_widget.height / 2)
    );
}
