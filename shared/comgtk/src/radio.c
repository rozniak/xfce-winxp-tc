#include <glib.h>
#include <gtk/gtk.h>

#include "../public/radio.h"

//
// PUBLIC FUNCTIONS
//
guint wintc_radio_group_get_selection(
    GSList* group
)
{
    GSList* iter = group;

    for (guint i = 0; iter; iter = iter->next, i++)
    {
        if (
            gtk_toggle_button_get_active(
                GTK_TOGGLE_BUTTON(iter->data)
            )
        )
        {
            // From GtkBuilder, it's in reverse
            //
            return g_slist_length(group) - i - 1;
        }
    }

    g_critical("%s", "comgtk: no radio in group was active");
    return 0;
}
