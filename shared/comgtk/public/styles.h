/** @file */

#ifndef __COMGTK_STYLES_H__
#define __COMGTK_STYLES_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Adds application priority CSS to the specified widget.
 *
 * @param widget The widget.
 * @param css    The CSS to add, which will have application priority.
 */
void wintc_widget_add_css(
    GtkWidget*   widget,
    const gchar* css
);

/**
 * Adds a style class to the specified widget.
 *
 * @param widget     The widget.
 * @param class_name The style class.
 */
void wintc_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
);

#endif
