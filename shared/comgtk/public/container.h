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

#endif
