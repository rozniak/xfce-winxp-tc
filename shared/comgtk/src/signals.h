#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
void wintc_signal_connect_list(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
);

#endif
