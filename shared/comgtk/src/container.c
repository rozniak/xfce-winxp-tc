#include <glib.h>
#include <gtk/gtk.h>

#include "../public/container.h"

//
// PUBLIC FUNCTIONS
//
void wintc_container_clear(
    GtkContainer* container
)
{
    GList* children = gtk_container_get_children(container);
    GList* iter     = children;

    while (iter)
    {
        gtk_widget_destroy(GTK_WIDGET(iter->data));

        iter = iter->next;
    }

    g_list_free(children);
}
