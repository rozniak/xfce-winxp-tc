#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "application.h"
#include "window.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SHEXT_HOST = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_window_constructed(
    GObject* object
);
static void wintc_explorer_window_dispose(
    GObject* object
);
static void wintc_explorer_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void prepare_new_location(
    const gchar*        specified_path,
    WinTCShextPathInfo* current_path_info
);

static void on_browser_location_changed(
    WinTCShBrowser* self,
    gpointer        user_data
);

static void on_button_nav_go_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_nav_up_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_combo_address_entry_activate(
    GtkEntry* self,
    gpointer  user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExplorerWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCExplorerWindow
{
    GtkApplicationWindow __parent__;

    // Shell stuff
    //
    WinTCShBrowser* browser;
    WinTCShextHost* shext_host;

    // UI
    //
    WinTCShIconViewBehaviour* behaviour_icons;
    GtkWidget*                iconview_browser;

    GtkWidget* button_nav_go;
    GtkWidget* button_nav_up;
    GtkWidget* combo_address;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExplorerWindow,
    wintc_explorer_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_explorer_window_class_init(
    WinTCExplorerWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_explorer_window_constructed;
    object_class->dispose      = wintc_explorer_window_dispose;
    object_class->set_property = wintc_explorer_window_set_property;

    g_object_class_install_property(
        object_class,
        PROP_SHEXT_HOST,
        g_param_spec_object(
            "shext-host",
            "ShextHost",
            "The shell extension host object to use.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_explorer_window_init(
    WinTCExplorerWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    // FIXME: Don't know what the default size should be
    //
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        500,
        350
    );

    // Initialize UI
    //
    g_type_ensure(WINTC_TYPE_CTL_ANIMATION);

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/explorer.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

    self->iconview_browser =
        GTK_WIDGET(gtk_builder_get_object(builder, "browse-view"));

    self->combo_address =
        GTK_WIDGET(gtk_builder_get_object(builder, "combo-address"));
    self->button_nav_go =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-nav-go"));
    self->button_nav_up =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-nav-up"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    // Link up UI
    // FIXME: This is temp, use GActions!
    //
    g_signal_connect(
        self->button_nav_go,
        "clicked",
        G_CALLBACK(on_button_nav_go_clicked),
        self
    );
    g_signal_connect(
        self->button_nav_up,
        "clicked",
        G_CALLBACK(on_button_nav_up_clicked),
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
static void wintc_explorer_window_constructed(
    GObject* object
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(object);

    if (!wnd->shext_host)
    {
        g_critical("%s", "explorer: window has no shext host");
        return;
    }

    // Set up shell browser
    //
    wnd->browser = wintc_sh_browser_new(wnd->shext_host);

    g_signal_connect(
        wnd->browser,
        "location-changed",
        G_CALLBACK(on_browser_location_changed),
        wnd
    );

    // Link up with UI
    //
    wnd->behaviour_icons =
        wintc_sh_icon_view_behaviour_new(
            GTK_ICON_VIEW(wnd->iconview_browser),
            wnd->browser
        );

    // Navigate to desktop
    // FIXME: Need to check if we were initialised with a path in future!
    //
    GError*            error     = NULL;
    WinTCShextPathInfo path_info = { 0 };

    path_info.base_path = wintc_sh_path_for_guid(WINTC_SH_GUID_DESKTOP);

    if (
        !wintc_sh_browser_set_location(
            wnd->browser,
            &path_info,
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error);
    }

    wintc_shext_path_info_free_data(&path_info);

    (G_OBJECT_CLASS(wintc_explorer_window_parent_class))->constructed(object);
}

static void wintc_explorer_window_dispose(
    GObject* object
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(object);

    g_clear_object(&(wnd->behaviour_icons));
    g_clear_object(&(wnd->browser));
    g_clear_object(&(wnd->shext_host));

    (G_OBJECT_CLASS(wintc_explorer_window_parent_class))->dispose(object);
}

static void wintc_explorer_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            wnd->shext_host = g_value_dup_object(value);
            break;

         default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_explorer_window_new(
    WinTCExplorerApplication* app,
    WinTCShextHost*           shext_host
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_EXPLORER_WINDOW,
            "application", GTK_APPLICATION(app),
            "shext-host",  shext_host,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void prepare_new_location(
    const gchar*        specified_path,
    WinTCShextPathInfo* current_path_info
)
{
    GError*       error            = NULL;
    const GRegex* regex_uri_scheme = wintc_regex_uri_scheme(&error);

    if (!regex_uri_scheme)
    {
        wintc_nice_error_and_clear(&error);
        return;
    }

    // If the path starts with '::' then assume it's a GUID and shouldn't be
    // touched
    //
    if (g_str_has_prefix(specified_path, "::"))
    {
        wintc_shext_path_info_free_data(current_path_info);

        current_path_info->base_path = g_strdup(specified_path);

        return;
    }

    // If the path starts with a leading slash '/' then assume an absolute file
    // system path
    //
    if (g_str_has_prefix(specified_path, "/"))
    {
        wintc_shext_path_info_free_data(current_path_info);

        current_path_info->base_path =
            g_strdup_printf("file://%s", specified_path);

        return;
    }

    // If the path is a URL, handle it here
    // FIXME: For now we just pass it on, in future we'll need to handle the
    //        scheme for cases like HTTP(S)
    //
    if (
        g_regex_match(
            regex_uri_scheme,
            specified_path,
            0,
            NULL
        )
    )
    {
        wintc_shext_path_info_free_data(current_path_info);

        current_path_info->base_path = g_strdup(specified_path);

        return;
    }

    // Here we assume it's a relative path
    // FIXME: When HTTP(S) is supported, we will need to check the current path
    //
    g_free(current_path_info->extended_path);

    current_path_info->extended_path = g_strdup(specified_path);
}

//
// CALLBACKS
//
static void on_browser_location_changed(
    WinTCShBrowser* self,
    gpointer        user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    // Just update the address bar for now
    //
    GtkWidget*         entry;
    WinTCShextPathInfo path_info = { 0 };

    entry =
        gtk_bin_get_child(
            GTK_BIN(wnd->combo_address)
        );

    wintc_sh_browser_get_location(self, &path_info);

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

static void on_button_nav_go_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    GtkWidget*         entry;
    GError*            error     = NULL;
    WinTCShextPathInfo path_info = { 0 };
    const gchar*       target_path;

    entry =
        gtk_bin_get_child(
            GTK_BIN(wnd->combo_address)
        );

    target_path =
        gtk_entry_get_text(GTK_ENTRY(entry));

    // Don't bother navigating if there's nowhere to go!
    //
    if (g_strcmp0(target_path, "") == 0)
    {
        return;
    }

    // Retrieve new location
    //
    wintc_sh_browser_get_location(
        wnd->browser,
        &path_info
    );

    prepare_new_location(
        target_path,
        &path_info
    );

    // Attempt the navigation
    //
    if (
        !wintc_sh_browser_set_location(
            wnd->browser,
            &path_info,
            &error
        )
    )
    {
        wintc_nice_error_and_clear(&error);
    }

    wintc_shext_path_info_free_data(&path_info);
}

static void on_button_nav_up_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    wintc_sh_browser_navigate_to_parent(wnd->browser);
}

static void on_combo_address_entry_activate(
    WINTC_UNUSED(GtkEntry* self),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    gtk_widget_activate(
        GTK_WIDGET(wnd->button_nav_go)
    );
}
