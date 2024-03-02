#include <glib.h>
#include <gtk/gtk.h>

#include "../public/signals.h"

//
// PUBLIC FUNCTIONS
//
void wintc_signal_connect_list(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
)
{
    GList* li;

    for (li = widgets; li != NULL; li = li->next)
    {
        g_assert(GTK_IS_WIDGET(li->data));

        g_signal_connect(
            G_OBJECT(li->data),
            signal_name,
            cb,
            user_data
        );
    }
}
