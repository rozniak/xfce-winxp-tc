#include <glib.h>
#include <gtk/gtk.h>

#include "../public/icons.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_icon_get_available_name(
    GIcon* icon
)
{
    // This can only work with themed icons
    //
    if (!icon || !G_IS_THEMED_ICON(icon))
    {
        return g_strdup("empty");
    }

    // Let's try to find an icon name present in the current theme
    //
    GtkIconTheme* icon_theme = gtk_icon_theme_get_default();

    const gchar* const* icon_names =
        g_themed_icon_get_names(G_THEMED_ICON(icon));

    for (gint i = 0; icon_names[i] != NULL; i++)
    {
        if (
            gtk_icon_theme_has_icon(
                icon_theme,
                icon_names[i]
            )
        )
        {
            return g_strdup(icon_names[i]);
        }
    }

    return g_strdup("empty");
}

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
