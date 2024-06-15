#ifndef __COMGTK_MEMORY_H__
#define __COMGTK_MEMORY_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
void wintc_freev(
    gpointer       mem,
    GDestroyNotify destroy
);

void wintc_freenv(
    gpointer       mem,
    guint          n_elements,
    GDestroyNotify destroy
);

#endif
