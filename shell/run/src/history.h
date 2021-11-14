#ifndef __HISTORY_H__
#define __HISTORY_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
GList* wintc_get_run_history(
    GError** out_error
);

gboolean wintc_append_run_history(
    const gchar* cmdline,
    GError**     out_error
);

#endif
