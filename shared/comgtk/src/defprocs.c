#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include "../public/defprocs.h"
#include "../public/shorthand.h"

//
// PUBLIC FUNCTIONS
//
void wintc_menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
)
{
    if (gtk_menu_item_get_submenu(GTK_MENU_ITEM(widget)) != NULL)
    {
        return;
    }

    gtk_menu_shell_deactivate(menu_shell);
}

void wintc_menu_shell_select_on_enter(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
)
{
    gtk_menu_shell_select_item(
        menu_shell,
        widget
    );
}
