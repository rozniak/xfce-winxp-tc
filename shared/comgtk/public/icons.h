#ifndef __COMGTK_ICONS_H__
#define __COMGTK_ICONS_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
gchar* wintc_icon_get_available_name(
    GIcon* icon
);
const gchar* wintc_icon_name_first_available(
    gint         size,
    const gchar* xdg_fallback,
    ...
);

#endif
