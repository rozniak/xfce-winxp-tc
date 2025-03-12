/** @file */

#ifndef __COMGTK_CONTAINER_H__
#define __COMGTK_CONTAINER_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Destroys all child widgets of a container.
 *
 * @param container The container.
 */
void wintc_container_clear(
    GtkContainer* container
);

/**
 * Gets the nth child widget of a container.
 *
 * @param container The container.
 * @param pos       The index of the widget.
 * @return The child widget at position N in the container.
 */
GtkWidget* wintc_container_get_nth_child(
    GtkContainer* container,
    gint          pos
);

#endif
