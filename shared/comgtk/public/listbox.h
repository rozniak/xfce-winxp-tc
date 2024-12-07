/** @file */

#ifndef __COMGTK_LISTBOX_H__
#define __COMGTK_LISTBOX_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Scrolls a list box to its selected item; this function is queued up as it
 * must be performed after any newly added rows are realised.
 *
 * @param list_box The list box.
 */
void wintc_list_box_queue_scroll_to_selected(
    GtkListBox* list_box
);

#endif
