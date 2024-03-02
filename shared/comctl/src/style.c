#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "../public/style.h"

//
// PUBLIC FUNCTIONS
//
void wintc_ctl_install_default_styles(void)
{
    static gboolean already_done = FALSE;

    if (already_done)
    {
        return;
    }

    GtkCssProvider* css_default = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_default,
        "/uk/oddmatics/wintc/comctl/default.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_default),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    already_done = TRUE;
}
