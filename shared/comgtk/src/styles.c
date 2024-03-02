#include <gtk/gtk.h>

#include "../public/styles.h"

//
// PUBLIC FUNCIONS
//
void wintc_widget_add_css(
    GtkWidget*   widget,
    const gchar* css
)
{
    GtkCssProvider*  provider = gtk_css_provider_new();
    GtkStyleContext* style    = gtk_widget_get_style_context(widget);

    gtk_css_provider_load_from_data(
        provider,
        css,
        -1,
        NULL
    );

    gtk_style_context_add_provider(
        style,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}

void wintc_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
)
{
    GtkStyleContext* styles = gtk_widget_get_style_context(widget);

    gtk_style_context_add_class(styles, class_name);
}
