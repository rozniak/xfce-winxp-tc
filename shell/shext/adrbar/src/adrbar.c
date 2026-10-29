#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "adrbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_address_constructed(
    GObject* object
);

static void cb_weak_ref_widget(
    gpointer user_data,
    GObject* where_the_object_was
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
    WinTCIShextShellWindow* shwnd,
    gpointer                user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarAddress
{
    WinTCShextUIController __parent__;

    WinTCIShextShellWindow* shwnd;

    // UI
    //
    GtkWidget* toolbar;

    GtkWidget* button_nav_go;
    GtkWidget* combo_address;
    GtkWidget* img_go;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarAddress,
    wintc_toolbar_address,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_address_class_init(
    WinTCToolbarAddressClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_address_constructed;

    // Add default styles
    //
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/toolbar/adrbar/appstyles.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
}

static void wintc_toolbar_address_init(
    WINTC_UNUSED(WinTCToolbarAddress* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_address_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_address_parent_class))
        ->constructed(object);

    WinTCToolbarAddress* ctl_adr = WINTC_TOOLBAR_ADDRESS(object);

    // Create the toolbar from builder
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/toolbar/adrbar/adrbar.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "toolbar",       &(ctl_adr->toolbar),
        "button-nav-go", &(ctl_adr->button_nav_go),
        "combo-address", &(ctl_adr->combo_address),
        "img-go",        &(ctl_adr->img_go),
        NULL
    );

    // Set the Go button icon with fallbacks
    //
    gchar* s_icon_names_go[] = {
        "wintc-go-button"
    };

    GIcon* icon_go =
        g_themed_icon_new_from_names(
            s_icon_names_go,
            G_N_ELEMENTS(s_icon_names_go)
        );

    gtk_image_set_from_gicon(
        GTK_IMAGE(ctl_adr->img_go),
        icon_go,
        GTK_ICON_SIZE_BUTTON
    );

    g_object_unref(icon_go);

    // Create toolbar in host shell window
    //
    WinTCIShextUIHost* ui_host =
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(ctl_adr)
        );

    ctl_adr->shwnd = WINTC_ISHEXT_SHELL_WINDOW(ui_host);

    wintc_ishext_ui_host_get_ext_widget(
        ui_host,
        WINTC_SHEXT_SHWND_HOSTEXT_TOOLBAR,
        GTK_TYPE_TOOLBAR,
        ctl_adr->toolbar
    );

    g_object_unref(builder);

    // Connect to location changed signal to update the entry
    //
    g_signal_connect(
        ctl_adr->button_nav_go,
        "clicked",
        G_CALLBACK(on_button_nav_go_clicked),
        ctl_adr
    );
    g_signal_connect(
        gtk_bin_get_child(GTK_BIN(ctl_adr->combo_address)),
        "activate",
        G_CALLBACK(on_combo_address_entry_activate),
        ctl_adr
    );

    g_signal_connect(
        ctl_adr->shwnd,
        "location-changed",
        G_CALLBACK(on_explorer_wnd_location_changed),
        ctl_adr
    );

    // Listen for when the toolbar is destroyed, and destroy ourselves with it
    //
    g_object_weak_ref(
        G_OBJECT(ctl_adr->toolbar),
        (GWeakNotify) cb_weak_ref_widget,
        ctl_adr
    );
}

//
// CALLBACKS
//
static void cb_weak_ref_widget(
    gpointer user_data,
    WINTC_UNUSED(GObject* where_the_object_was)
)
{
    g_object_unref(G_OBJECT(user_data));
}

static void on_button_nav_go_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCToolbarAddress* ctl_adr = WINTC_TOOLBAR_ADDRESS(user_data);

    GtkWidget* entry = gtk_bin_get_child(GTK_BIN(ctl_adr->combo_address));

    wintc_ishext_shell_window_navigate_simple(
        ctl_adr->shwnd,
        gtk_entry_get_text(GTK_ENTRY(entry)),
        NULL // FIXME: Error handling
    );
}

static void on_combo_address_entry_activate(
    WINTC_UNUSED(GtkEntry* self),
    gpointer user_data
)
{
    WinTCToolbarAddress* ctl_adr = WINTC_TOOLBAR_ADDRESS(user_data);

    gtk_widget_activate(
        GTK_WIDGET(ctl_adr->button_nav_go)
    );
}

static void on_explorer_wnd_location_changed(
    WinTCIShextShellWindow* shwnd,
    gpointer                user_data
)
{
    WinTCToolbarAddress* ctl_adr = WINTC_TOOLBAR_ADDRESS(user_data);

    GtkWidget*         entry     = gtk_bin_get_child(
                                       GTK_BIN(ctl_adr->combo_address)
                                   );
    WinTCShextPathInfo path_info = { 0 };

    wintc_ishext_shell_window_get_location(
        shwnd,
        &path_info
    );

    if (
        path_info.extended_path &&
        strlen(path_info.extended_path) > 0
    )
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
