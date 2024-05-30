#include <glib.h>
#include <gtk/gtk.h>

#include "../public/icons.h"

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_icon_name_first_available(
    gint         size,
    const gchar* xdg_fallback,
    ...
)
{
    va_list      ap;
    const gchar* next_name;
    const gchar* ret = xdg_fallback;

    va_start(ap, xdg_fallback);

    next_name = va_arg(ap, gchar*);

    while (next_name)
    {
        GtkIconInfo* icon_info =
            gtk_icon_theme_lookup_icon(
                gtk_icon_theme_get_default(),
                next_name,
                size,
                0
            );

        if (icon_info)
        {
            g_object_unref(icon_info);

            ret = next_name;

            break;
        }

        next_name = va_arg(ap, gchar*);
    }

    va_end(ap);

    return ret;
}
