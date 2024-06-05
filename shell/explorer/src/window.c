#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "application.h"
#include "toolbar.h"
#include "toolbars/adrbar.h"
#include "toolbars/stdbar.h"
#include "window.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SHEXT_HOST = 1
};

enum
{
    SIGNAL_LOCATION_CHANGED = 0,
    N_SIGNALS
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_window_constructed(
    GObject* object
);
static void wintc_explorer_window_finalize(
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

static void do_navigation(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
);
static void prepare_new_location(
    const gchar*        specified_path,
    WinTCShextPathInfo* current_path_info
);

static void action_nav_go(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_nav_up(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void on_browser_location_changed(
    WinTCShBrowser* self,
    gpointer        user_data
);

//
// STATIC DATA
//
static GActionEntry s_window_actions[] = {
    {
        .name           = "nav-go",
        .activate       = action_nav_go,
        .parameter_type = "s",
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "nav-up",
        .activate       = action_nav_up,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

static gint wintc_explorer_window_signals[N_SIGNALS] = { 0 };

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

    // State
    //
    WinTCShextPathInfo current_path;

    WinTCExplorerToolbar* toolbar_adr;
    WinTCExplorerToolbar* toolbar_std;

    // UI
    //
    WinTCShIconViewBehaviour* behaviour_icons;
    GtkWidget*                iconview_browser;

    GtkWidget* box_toolbars;
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
    object_class->finalize     = wintc_explorer_window_finalize;
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

    wintc_explorer_window_signals[SIGNAL_LOCATION_CHANGED] =
        g_signal_new(
            "location-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_explorer_window_init(
    WinTCExplorerWindow* self
)
{
    GtkBuilder* builder;
    GtkWidget*  main_box;

    // Define GActions
    //
    g_action_map_add_action_entries(
        G_ACTION_MAP(self),
        s_window_actions,
        G_N_ELEMENTS(s_window_actions),
        self
    );

    // FIXME: Defaulting nav-up to disabled, in future this will depend on the
    //        initial location of the explorer window
    //
    g_simple_action_set_enabled(
        G_SIMPLE_ACTION(
            g_action_map_lookup_action(
                G_ACTION_MAP(self),
                "nav-up"
            )
        ),
        FALSE
    );

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

    self->box_toolbars =
        GTK_WIDGET(gtk_builder_get_object(builder, "box-toolbars"));

    gtk_container_add(GTK_CONTAINER(self), main_box);

    g_object_unref(builder);

    // FIXME: Toolbars are configurable!
    //
    self->toolbar_adr =
        wintc_exp_address_toolbar_new(self);
    self->toolbar_std =
        wintc_exp_standard_toolbar_new(self);

    gtk_container_add(
        GTK_CONTAINER(self->box_toolbars),
        wintc_explorer_toolbar_get_toolbar(self->toolbar_std)
    );
    gtk_container_add(
        GTK_CONTAINER(self->box_toolbars),
        wintc_explorer_toolbar_get_toolbar(self->toolbar_adr)
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
    do_navigation(
        wnd,
        wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
    );

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

    g_clear_object(&(wnd->toolbar_adr));
    g_clear_object(&(wnd->toolbar_std));

    (G_OBJECT_CLASS(wintc_explorer_window_parent_class))->dispose(object);
}

static void wintc_explorer_window_finalize(
    GObject* object
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(object);

    wintc_shext_path_info_free_data(&(wnd->current_path));

    (G_OBJECT_CLASS(wintc_explorer_window_parent_class))->finalize(object);
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

void wintc_explorer_window_get_location(
    WinTCExplorerWindow* wnd,
    WinTCShextPathInfo*  path_info
)
{
    if (!wnd->current_path.base_path)
    {
        return;
    }

    wintc_shext_path_info_copy(
        path_info,
        &(wnd->current_path)
    );
}

//
// PRIVATE FUNCTIONS
//
static void do_navigation(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
)
{
    GError*            error     = NULL;
    WinTCShextPathInfo path_info = { 0 };

    // Don't bother navigating if there's nowhere to go!
    //
    if (g_strcmp0(specified_path, "") == 0)
    {
        return;
    }

    // Retrieve new location
    //
    wintc_shext_path_info_copy(
        &path_info,
        &(wnd->current_path)
    );

    prepare_new_location(
        specified_path,
        &path_info
    );

    // Attempt the navigation
    //
    if (
        wintc_sh_browser_set_location(
            wnd->browser,
            &path_info,
            &error
        )
    )
    {
        wintc_shext_path_info_move(
            &(wnd->current_path),
            &path_info
        );

        g_signal_emit(
            wnd,
            wintc_explorer_window_signals[SIGNAL_LOCATION_CHANGED],
            0
        );
    }
    else
    {
        wintc_nice_error_and_clear(&error);
    }

    wintc_shext_path_info_free_data(&path_info);
}

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
static void action_nav_go(
    WINTC_UNUSED(GSimpleAction* action),
    GVariant* parameter,
    gpointer  user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    do_navigation(
        wnd,
        g_variant_get_string(parameter, NULL)
    );
}

static void action_nav_up(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    wintc_sh_browser_navigate_to_parent(wnd->browser);
}

static void on_browser_location_changed(
    WinTCShBrowser* self,
    gpointer        user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    // Update our local state and emit on the window
    //
    wintc_shext_path_info_free_data(&(wnd->current_path));

    wintc_sh_browser_get_location(
        wnd->browser,
        &(wnd->current_path)
    );

    g_signal_emit(
        wnd,
        wintc_explorer_window_signals[SIGNAL_LOCATION_CHANGED],
        0
    );

    // Update action(s)
    //
    g_simple_action_set_enabled(
        G_SIMPLE_ACTION(
            g_action_map_lookup_action(
                G_ACTION_MAP(wnd),
                "nav-up"
            )
        ),
        wintc_sh_browser_can_navigate_to_parent(self)
    );
}
