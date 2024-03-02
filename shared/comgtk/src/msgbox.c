#include <gtk/gtk.h>

#include "../public/msgbox.h"

//
// PUBLIC FUNCTIONS
//
gint wintc_messagebox_show(
    GtkWindow*     parent,
    const gchar*   text,
    const gchar*   caption,
    GtkButtonsType buttons,
    GtkMessageType type
)
{
    GtkWidget* dialog =
        gtk_message_dialog_new(
            parent,
            GTK_DIALOG_MODAL,
            type,
            buttons,
            "%s",
            text
        );

    gtk_window_set_title(GTK_WINDOW(dialog), caption);

    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    return response;
}
