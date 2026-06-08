/** @file */

#ifndef __COMGTK_WIDGET_H__
#define __COMGTK_WIDGET_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Sets the widget's size request equal to its preferred natural size. This is
 * useful for fixing up labels with wrapping.
 *
 * @param widget The widget.
 */
void wintc_widget_set_size_request_natural(
    GtkWidget* widget
);

#endif
