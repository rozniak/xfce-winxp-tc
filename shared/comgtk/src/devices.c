#include <glib.h>
#include <gtk/gtk.h>

#include "../public/devices.h"

//
// PUBLIC FUNCTIONS
//
void wintc_widget_get_modifier_mask(
    GtkWidget*       widget,
    GdkModifierType* mask
)
{
    GdkWindow*  wnd     = gtk_widget_get_window(widget);
    GdkDisplay* display = gdk_window_get_display(wnd);
    GdkSeat*    seat    = gdk_display_get_default_seat(display);
    GdkDevice*  pointer = gdk_seat_get_pointer(seat);

    gdk_window_get_device_position(
        wnd,
        pointer,
        NULL,
        NULL,
        mask
    );
}
