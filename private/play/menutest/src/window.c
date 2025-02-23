#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// STATIC DATA
//
static gchar* S_MENU_ITEMS[] = {
    "Item 1", "folder",
    "Item 2", "computer",
    "Item 3", "add"
};

static gchar* S_SUBMENU_ITEMS[] = {
    "Subitem 1", "search",
    "Subitem 2", "printer",
    "Subitem 3", "drive-optical"
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCMenuTestWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCMenuTestWindow
{
    GtkApplicationWindow __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCMenuTestWindow,
    wintc_menu_test_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_menu_test_window_class_init(
    WINTC_UNUSED(WinTCMenuTestWindowClass* klass)
) {}

static void wintc_menu_test_window_init(
    WinTCMenuTestWindow* self
)
{
    GtkWidget* box_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    // Create a boring menu strip
    //
    GtkWidget* menu_bar       = gtk_menu_bar_new();
    GtkWidget* menu_item_host = gtk_menu_item_new_with_label("Menu");

    gtk_menu_shell_append(
        GTK_MENU_SHELL(menu_bar),
        menu_item_host
    );

    gtk_container_add(GTK_CONTAINER(box_container), menu_bar);

    // Create a GMenu structure...
    //
    GMenu* menu = g_menu_new();

    // ...start with the submenu
    //
    GMenu*     submenu      = g_menu_new();
    GMenuItem* submenu_item = g_menu_item_new(NULL, NULL);

    for (gsize i = 0; i < G_N_ELEMENTS(S_SUBMENU_ITEMS); i += 2)
    {
        GMenuItem* menu_item = g_menu_item_new(NULL, NULL);

        g_menu_item_set_label(
            menu_item,
            S_SUBMENU_ITEMS[i]
        );
        g_menu_item_set_icon(
            menu_item,
            g_themed_icon_new(S_SUBMENU_ITEMS[i + 1])
        );

        g_menu_append_item(submenu, menu_item);

        g_object_unref(menu_item);
    }

    g_menu_item_set_label(
        submenu_item,
        "Submenu"
    );
    g_menu_item_set_icon(
        submenu_item,
        g_themed_icon_new("applications-other")
    );
    g_menu_item_set_submenu(
        submenu_item,
        G_MENU_MODEL(submenu)
    );

    g_menu_append_item(menu, submenu_item);

    g_object_unref(submenu_item);

    // ..create the rest of the menu items
    //
    for (gsize i = 0; i < G_N_ELEMENTS(S_MENU_ITEMS); i += 2)
    {
        GMenuItem* menu_item = g_menu_item_new(NULL, NULL);

        g_menu_item_set_label(
            menu_item,
            S_MENU_ITEMS[i]
        );
        g_menu_item_set_icon(
            menu_item,
            g_themed_icon_new(S_MENU_ITEMS[i + 1])
        );

        g_menu_append_item(menu, menu_item);

        g_object_unref(menu_item);
    }

    // Create GTK menu, bind model, attach as submenu
    //
    GtkWidget* bound_submenu = gtk_menu_new_from_model(G_MENU_MODEL(menu));

    gtk_menu_item_set_submenu(
        GTK_MENU_ITEM(menu_item_host),
        bound_submenu
    );

    // Show!
    //
    gtk_container_add(GTK_CONTAINER(self), box_container);
    gtk_widget_show_all(box_container);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_menu_test_window_new(
    WinTCMenuTestApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_MENU_TEST_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Hello Windows!",
            NULL
        )
    );
}
