#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void on_button_load_clicked(
    GtkButton* self,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCIconDbgWindow
{
    GtkApplicationWindow __parent__;

    // UI
    //
    GtkWidget* entry_icon_name;
    GtkWidget* combo_size;
    GtkWidget* img_icon;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCIconDbgWindow,
    wintc_icon_dbg_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_icon_dbg_window_class_init(
    WINTC_UNUSED(WinTCIconDbgWindowClass* klass)
) {}

static void wintc_icon_dbg_window_init(
    WinTCIconDbgWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  button_load;
    GtkWidget*  paned_root;

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        400,
        300
    );

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/icondbg/icondbg.ui"
        );

    wintc_builder_get_objects(
        builder,
        "button-load",     &button_load,
        "entry-icon-name", &(self->entry_icon_name),
        "combo-size",      &(self->combo_size),
        "img-icon",        &(self->img_icon),
        "paned-root",      &paned_root,
        NULL
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        GTK_WIDGET(gtk_builder_get_object(builder, "paned-root"))
    );

    g_signal_connect(
        button_load,
        "clicked",
        G_CALLBACK(on_button_load_clicked),
        self
    );

    g_object_unref(builder);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_icon_dbg_window_new(
    WinTCIconDbgApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_ICON_DBG_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "Icon Viewer",
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_button_load_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCIconDbgWindow* wnd = WINTC_ICON_DBG_WINDOW(user_data);

    // Retrieve the details on what we need to load
    //
    const gchar* icon_name;
    gint         icon_size;

    icon_name = gtk_entry_get_text(GTK_ENTRY(wnd->entry_icon_name));
    icon_size = gtk_combo_box_get_active(GTK_COMBO_BOX(wnd->combo_size));
    icon_size = (icon_size + 1) * 16;

    // Attempt to load the icon
    //
    GError*    error       = NULL;
    GdkPixbuf* pixbuf_icon =
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            icon_name,
            icon_size,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            &error
        );

    if (!pixbuf_icon)
    {
        wintc_display_error_and_clear(&error, GTK_WINDOW(wnd));
        return;
    }

    gtk_image_set_from_pixbuf(
        GTK_IMAGE(wnd->img_icon),
        pixbuf_icon
    );

    g_object_unref(pixbuf_icon);
}
