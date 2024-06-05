#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../toolbar.h"
#include "../window.h"
#include "adrbar.h"

//
// FOWARD DECLARATIONS
//
static void wintc_exp_address_toolbar_constructed(
    GObject* object
);

static void on_button_nav_go_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_combo_address_entry_activate(
    GtkEntry* self,
    gpointer  user_data
);
static void on_explorer_wnd_location_changed(
    WinTCExplorerWindow* wnd,
    gpointer             user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpAddressToolbarClass
{
    WinTCExplorerToolbarClass __parent__;
};

struct _WinTCExpAddressToolbar
{
    WinTCExplorerToolbar __parent__;

    // UI
    //
    GtkWidget* button_nav_go;
    GtkWidget* combo_address;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpAddressToolbar,
    wintc_exp_address_toolbar,
    WINTC_TYPE_EXPLORER_TOOLBAR
)

static void wintc_exp_address_toolbar_class_init(
    WinTCExpAddressToolbarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_address_toolbar_constructed;
}

static void wintc_exp_address_toolbar_init(
    WinTCExpAddressToolbar* self
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(self);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/adrbar.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    // Replace toolbar with the built one
    //
    g_clear_object(&(toolbar->toolbar));

    toolbar->toolbar =
        GTK_WIDGET(
            g_object_ref_sink(
                gtk_builder_get_object(builder, "toolbar")
            )
        );

    // Pull out other widgets we need
    //
    self->button_nav_go =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-nav-go"));
    self->combo_address =
        GTK_WIDGET(gtk_builder_get_object(builder, "combo-address"));

    g_object_unref(builder);

    // Connect signals
    //
    g_signal_connect(
        self->button_nav_go,
        "clicked",
        G_CALLBACK(on_button_nav_go_clicked),
        self
    );
    g_signal_connect(
        gtk_bin_get_child(GTK_BIN(self->combo_address)),
        "activate",
        G_CALLBACK(on_combo_address_entry_activate),
        self
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_address_toolbar_constructed(
    GObject* object
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(object);

    // Connect to location changed signal to update the entry
    //
    g_signal_connect(
        toolbar->owner_explorer_wnd,
        "location_changed",
        G_CALLBACK(on_explorer_wnd_location_changed),
        object
    );

    (G_OBJECT_CLASS(wintc_exp_address_toolbar_parent_class))
        ->constructed(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerToolbar* wintc_exp_address_toolbar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_TOOLBAR(
        g_object_new(
            WINTC_TYPE_EXP_ADDRESS_TOOLBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_button_nav_go_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCExplorerToolbar*   toolbar     = WINTC_EXPLORER_TOOLBAR(user_data);
    WinTCExpAddressToolbar* toolbar_adr = WINTC_EXP_ADDRESS_TOOLBAR(user_data);

    GtkWidget* entry = gtk_bin_get_child(GTK_BIN(toolbar_adr->combo_address));

    g_action_activate(
        g_action_map_lookup_action(
            G_ACTION_MAP(toolbar->owner_explorer_wnd),
            "nav-go"
        ),
        g_variant_new_string(
            gtk_entry_get_text(GTK_ENTRY(entry))
        )
    );
}

static void on_combo_address_entry_activate(
    WINTC_UNUSED(GtkEntry* self),
    gpointer user_data
)
{
    WinTCExpAddressToolbar* toolbar_adr = WINTC_EXP_ADDRESS_TOOLBAR(user_data);

    gtk_widget_activate(
        GTK_WIDGET(toolbar_adr->button_nav_go)
    );
}

static void on_explorer_wnd_location_changed(
    WinTCExplorerWindow* wnd,
    gpointer             user_data
)
{
    WinTCExpAddressToolbar* toolbar_adr = WINTC_EXP_ADDRESS_TOOLBAR(user_data);

    GtkWidget*         entry     = gtk_bin_get_child(
                                       GTK_BIN(toolbar_adr->combo_address)
                                   );
    WinTCShextPathInfo path_info = { 0 };

    wintc_explorer_window_get_location(
        wnd,
        &path_info
    );

    if (path_info.extended_path)
    {
        gtk_entry_set_text(
            GTK_ENTRY(entry),
            path_info.extended_path
        );
    }
    else
    {
        // Special case for file:// address, we only want to display the actual
        // filesystem path - kinda cheeky bunging it in here but it does get
        // the job done
        //
        if (g_str_has_prefix(path_info.base_path, "file://"))
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry),
                path_info.base_path + strlen("file://")
            );
        }
        else
        {
            gtk_entry_set_text(
                GTK_ENTRY(entry),
                path_info.base_path
            );
        }
    }

    wintc_shext_path_info_free_data(&path_info);
}
