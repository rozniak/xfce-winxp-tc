#include <glib.h>
#include <gtk/gtk.h>
#include <stdarg.h>

#include "../public/builder.h"

//
// PUBLIC FUNCTIONS
//
void wintc_builder_get_objects(
    GtkBuilder* builder,
    ...
)
{
    va_list     ap;
    GtkWidget** next_dst;
    gchar*      next_name;

    va_start(ap, builder);

    next_name = va_arg(ap, gchar*);

    while (next_name)
    {
        next_dst  = va_arg(ap, GtkWidget**);
        *next_dst = GTK_WIDGET(gtk_builder_get_object(builder, next_name));

        next_name = va_arg(ap, gchar*);
    }

    va_end(ap);
}

void wintc_widget_printf(
    GtkWidget* widget,
    ...
)
{
    GObjectClass* object_class = G_OBJECT_GET_CLASS(widget);
    const gchar*  property     = NULL;

    if (g_object_class_find_property(object_class, "label"))
    {
        property = "label";
    }
    else if (g_object_class_find_property(object_class, "title"))
    {
        property = "title";
    }
    else
    {
        g_critical("Widget does not support printf: %p", widget);
        return;
    }

    // Perform the printf
    //
    va_list ap;
    gchar*  format_string = NULL;
    gchar*  new_str;

    va_start(ap, widget);

    g_object_get(
        widget,
        property, &format_string,
        NULL
    );

    new_str = g_strdup_vprintf(format_string, ap);

    g_object_set(
        widget,
        property, new_str,
        NULL
    );

    va_end(ap);
    g_free(format_string);
    g_free(new_str);
}
