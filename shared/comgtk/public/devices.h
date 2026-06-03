/** @file */

#ifndef __COMGTK_DEVICES_H__
#define __COMGTK_DEVICES_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Gets the pointer modifier state on a widget's window.
 *
 * @param widget The widget.
 * @param mask   The return location for the modifier state mask.
 */
void wintc_widget_get_modifier_mask(
    GtkWidget*       widget,
    GdkModifierType* mask
);

#endif
