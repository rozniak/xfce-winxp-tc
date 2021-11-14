#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <glib.h>

#define WINTC_COMPONENT_SHELL "shell"

//
// PUBLIC FUNCTIONS
//
gboolean wintc_profile_ensure_exists(
    const gchar* component,
    GError**     out_error
);

gchar* wintc_profile_get_path(
    const gchar* component,
    const gchar* filename
);

gboolean wintc_profile_get_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar**      contents,
    gsize*       length,
    GError**     error
);

gboolean wintc_profile_set_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar*       contents,
    gssize       length,
    GError**     error
);

#endif
