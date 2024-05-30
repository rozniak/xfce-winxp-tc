#ifndef __COMGTK_ICONS_H__
#define __COMGTK_ICONS_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_icon_name_first_available(
    gint         size,
    const gchar* xdg_fallback,
    ...
);

#endif
