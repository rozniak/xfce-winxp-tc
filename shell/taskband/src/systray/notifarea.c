#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "clock.h"
#include "notifarea.h"

//
// PUBLIC FUNCTIONS
//
GtkWidget* notification_area_new(void)
{
    GtkWidget* box   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* clock = tray_clock_new();

    wintc_widget_add_style_class(box, "wintc-systray");

    gtk_box_pack_start(
        GTK_BOX(box),
        clock,
        FALSE,
        FALSE,
        0
    );

    return box;
}
