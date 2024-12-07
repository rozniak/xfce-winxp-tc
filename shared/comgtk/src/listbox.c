#include <glib.h>
#include <gtk/gtk.h>

#include "../public/debug.h"
#include "../public/listbox.h"
#include "../public/shorthand.h"

//
// FORWARD DECLARATIONS
//
static gboolean cb_list_box_scroll_to_selected(
    gpointer user_data
);

static void on_list_box_realize(
    GtkWidget* self,
    gpointer   user_data
);

//
// PUBLIC FUNCTIONS
//
void wintc_list_box_queue_scroll_to_selected(
    GtkListBox* list_box
)
{
    // If the list box itself isn't even realised then we must delay until
    // then
    //
    if (gtk_widget_get_realized(GTK_WIDGET(list_box)))
    {
        g_idle_add(
            (GSourceFunc) cb_list_box_scroll_to_selected,
            list_box
        );
    }
    else
    {
        g_signal_connect(
            list_box,
            "realize",
            G_CALLBACK(on_list_box_realize),
            NULL
        );
    }
}

//
// CALLBACKS
//
static gboolean cb_list_box_scroll_to_selected(
    gpointer user_data
)
{
    GtkListBox* list_box = GTK_LIST_BOX(user_data);

    // Find scrolled window and selected item...
    //
    GtkListBoxRow* row       = gtk_list_box_get_selected_row(list_box);
    GtkWidget*     scrollwnd = gtk_widget_get_ancestor(
                                   GTK_WIDGET(list_box),
                                   GTK_TYPE_SCROLLED_WINDOW
                               );

    if (!scrollwnd)
    {
        g_critical(
            "%s",
            "Can't scroll list box, it's not in a scrolled window."
        );

        return G_SOURCE_REMOVE;
    }

    if (!row)
    {
        WINTC_LOG_DEBUG("Not scrolling list box - nothing selected.");
        return G_SOURCE_REMOVE;
    }

    // Perform the scroll
    //
    gint scroll_y = 0;

    if (
        gtk_widget_translate_coordinates(
            GTK_WIDGET(row),
            GTK_WIDGET(list_box),
            0,
            0,
            NULL,
            &scroll_y
        )
    )
    {
        gtk_adjustment_set_value(
            gtk_scrolled_window_get_vadjustment(
                GTK_SCROLLED_WINDOW(scrollwnd)
            ),
            (gdouble) scroll_y
        );
    }

    return G_SOURCE_REMOVE;
}

static void on_list_box_realize(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    g_idle_add(
        (GSourceFunc) cb_list_box_scroll_to_selected,
        GTK_LIST_BOX(self)
    );

    // Disconnect the signal, only needed this once
    //
    g_signal_handlers_disconnect_by_func(
        self,
        on_list_box_realize,
        NULL
    );
}
