#ifndef __COMGTK_BUILDER_H__
#define __COMGTK_BUILDER_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieve many items at once from a GtkBuilder.
 *
 * @param builder The builder instance.
 * @param ...     Object IDs and locations to store them, terminated by NULL.
 */
void wintc_builder_get_objects(
    GtkBuilder* builder,
    ...
);

/**
 * Rewrite a widget's label, assuming its existing label is a printf format
 * string.
 *
 * @param widget The widget.
 * @param ...    The arguments to the printf.
 */
void wintc_widget_printf(
    GtkWidget* widget,
    ...
);

#endif
