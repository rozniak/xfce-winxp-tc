/** @file */

#ifndef __COMGTK_SIGNALS_H__
#define __COMGTK_SIGNALS_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Connects a callback to a signals on many widgets at once.
 *
 * @param widgets     The list of widgets.
 * @param signal_name The signal name.
 * @param cb          The callback to connect.
 * @param user_data   Data that will be passed to the callback.
 */
void wintc_signal_connect_list(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
);

#endif
