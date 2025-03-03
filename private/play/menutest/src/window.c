#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

// Change this to use either our binding or GTK
//
#define TEST_USE_CTL_BINDING 1

//
// FORWARD DECLARATIONS
//
static void wintc_menu_test_window_dispose(
    GObject* object
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
