#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

// Change this to use either our binding or GTK
//
#define TEST_USE_CTL_BINDING 0

//
// FORWARD DECLARATIONS
//
static void wintc_menu_test_window_dispose(
    GObject* object
);

static gint wintc_menu_test_window_find_item_index(
    GMenuModel*  model,
    GMenuModel** target_model
);

static void on_button_add_clicked(
    GtkButton* button,
    gpointer   user_data
);
static void on_button_del_clicked(
    GtkButton* button,
    gpointer   user_data
);

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

    WinTCCtlMenuBinding* menu_binding;
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
    WinTCMenuTestWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_menu_test_window_dispose;
}

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

    // Retrieve menu model
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/samples/menutest/menubar.ui"
        );

    GMenuModel* model =
        G_MENU_MODEL(gtk_builder_get_object(builder, "menu"));

    g_object_ref(model);

    g_object_unref(builder);

    // Create a boring menu strip
    //
    GtkWidget* menu_bar = gtk_menu_bar_new();

    if (TEST_USE_CTL_BINDING)
    {
        self->menu_binding =
            wintc_ctl_menu_binding_new(
                GTK_MENU_SHELL(menu_bar),
                model
            );
    }
    else
    {
        gtk_menu_shell_bind_model(
            GTK_MENU_SHELL(menu_bar),
            model,
            NULL,
            FALSE
        );
    }

    gtk_container_add(GTK_CONTAINER(box_container), menu_bar);

    // Add a strip for buttons for doing random additions/deletions
    //
    GtkWidget* box_buttons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GtkWidget* button_add = gtk_button_new_with_label("Random add");
    GtkWidget* button_del = gtk_button_new_with_label("Random del");

    gtk_box_pack_start(
        GTK_BOX(box_buttons),
        button_add,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(box_buttons),
        button_del,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(GTK_CONTAINER(box_container), box_buttons);

    g_signal_connect(
        button_add,
        "clicked",
        G_CALLBACK(on_button_add_clicked),
        model
    );
    g_signal_connect(
        button_del,
        "clicked",
        G_CALLBACK(on_button_del_clicked),
        model
    );

    // Show!
    //
    gtk_container_add(GTK_CONTAINER(self), box_container);
    gtk_widget_show_all(box_container);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_menu_test_window_dispose(
    GObject* object
)
{
    WinTCMenuTestWindow* wnd = WINTC_MENU_TEST_WINDOW(object);

    if (wnd->menu_binding)
    {
        g_clear_object(&(wnd->menu_binding));
    }

    (G_OBJECT_CLASS(wintc_menu_test_window_parent_class))
        ->dispose(object);
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
            "title",       "Menu Binding Test",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gint wintc_menu_test_window_find_item_index(
    GMenuModel*  model,
    GMenuModel** target_model
)
{
    // We receive a top level model (the menu bar) - pick a menu to work on
    //
    gint n_items = g_menu_model_get_n_items(model);

    GMenuModel* menu =
        g_menu_model_get_item_link(
            model,
            g_random_int_range(0, n_items),
            G_MENU_LINK_SUBMENU
        );

    // Find an item
    //
    gint        idx_rand;
    GMenuModel* model_current = menu;

    while (TRUE)
    {
        n_items = g_menu_model_get_n_items(model_current);

        // 0 items? It's over
        //
        if (!n_items)
        {
            *target_model = model_current;
            return 0;
        }

        // Wahey we have items, let's pick one
        //
        idx_rand = g_random_int_range(0, n_items);

        // If we hit a submenu, enter it
        //
        GMenuModel* model_submenu =
            g_menu_model_get_item_link(
                model_current,
                idx_rand,
                G_MENU_LINK_SUBMENU
            );

        if (model_submenu)
        {
            model_current = model_submenu;
            continue;
        }

        // Otherwise return this index
        //
        *target_model = model_current;
        return idx_rand;
    }
}

//
// CALLBACKS
//
static void on_button_add_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    GMenuModel* model = G_MENU_MODEL(user_data);

    gint        target_idx;
    GMenuModel* target_model = NULL;

    target_idx =
        wintc_menu_test_window_find_item_index(model, &target_model);

    // Create and add the menu item
    //
    GMenuItem* menu_item = g_menu_item_new("A random item.", NULL);

    g_menu_item_set_icon(
        menu_item,
        g_themed_icon_new("emblem-favorite")
    );

    g_menu_insert_item(G_MENU(target_model), target_idx, menu_item);

    g_object_unref(menu_item);
}

static void on_button_del_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    GMenuModel* model = G_MENU_MODEL(user_data);

    gint        target_idx;
    GMenuModel* target_model = NULL;

    // We shouldn't delete section items... this is really rubbish but I'm also
    // REALLY lazy so just try a few times to find a normal item otherwise give
    // up
    //
    gint attempts = 0;

    while (attempts < 3)
    {
        attempts++;

        target_idx =
            wintc_menu_test_window_find_item_index(model, &target_model);

        // Skip if this item is a section
        //
        if (
            g_menu_model_get_item_link(
                target_model,
                target_idx,
                G_MENU_LINK_SECTION
            )
        )
        {
            continue;
        }

        // Don't delete if there's actually no items in this model
        //
        if (!g_menu_model_get_n_items(target_model))
        {
            continue;
        }

        // All good, bin the item
        //
        g_menu_remove(G_MENU(target_model), target_idx);
        return;
    }

    g_message(
        "%s",
        "Didn't land on a normal item to bin, better luck next time."
    );
}
