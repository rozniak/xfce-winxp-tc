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

static void on_button_nav_up_clicked(
    GtkButton* self,
    gpointer   user_data
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

    GtkWidget* button_nav_up;
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

    self->button_nav_up =
        GTK_WIDGET(gtk_builder_get_object(builder, "button-nav-up"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    // Link up UI
    // FIXME: This is temp, use GActions!
    //
    g_signal_connect(
        self->button_nav_up,
        "clicked",
        G_CALLBACK(on_button_nav_up_clicked),
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
    WinTCExplorerWindow*      wnd = WINTC_EXPLORER_WINDOW(object);

    if (!wnd->shext_host)
    {
        g_critical("%s", "explorer: window has no shext host");
        return;
    }

    // Set up shell browser
    //
    wnd->browser = wintc_sh_browser_new(wnd->shext_host);

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
    gchar*  desktop_path = g_strdup_printf("::{%s}", WINTC_SH_GUID_DESKTOP);
    GError* error        = NULL;

    wintc_sh_browser_set_location(
        wnd->browser,
        desktop_path,
        &error
    );

    if (error)
    {
        wintc_display_error_and_clear(&error);
    }

    g_free(desktop_path);

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
// CALLBACKS
//
static void on_button_nav_up_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    wintc_sh_browser_navigate_to_parent(wnd->browser);
}
