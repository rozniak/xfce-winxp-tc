#ifndef __COMGTK_RADIO_H__
#define __COMGTK_RADIO_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieve the index of the active selection in a radio button group.
 *
 * @param group The radio button group.
 */
guint wintc_radio_group_get_selection(
    GSList* group
);

#endif
