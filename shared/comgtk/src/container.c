#include <glib.h>
#include <gtk/gtk.h>

#include "../public/container.h"

//
// PUBLIC FUNCTIONS
//
void wintc_container_clear(
    GtkContainer* container,
    gboolean      destroy
)
{
    GList* children = gtk_container_get_children(container);
    GList* iter     = children;

    if (destroy)
    {
        for (; iter; iter = iter->next)
        {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
    }
    else
    {
        for (; iter; iter = iter->next)
        {
            gtk_container_remove(
                container,
                GTK_WIDGET(iter->data)
            );
        }
    }

    g_list_free(children);
}

GtkWidget* wintc_container_get_nth_child(
    GtkContainer* container,
    gint          pos
)
{
    GList* children = gtk_container_get_children(container);

    GtkWidget* nth_child =
        GTK_WIDGET(g_list_nth_data(children, pos));

    g_list_free(children);

    return nth_child;
}
