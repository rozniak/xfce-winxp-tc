#include <canberra.h>
#include <canberra-gtk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
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
    PROP_SHEXT_HOST = 1,
    PROP_INITIAL_PATH
};

enum
{
    SIGNAL_LOCATION_CHANGED = 0,
    SIGNAL_MODE_CHANGED,
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
static void do_navigation_internet(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
);
static void do_navigation_local(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
);
static void prepare_new_location(
    const gchar*        specified_path,
    WinTCShextPathInfo* local_path_info
);
static void switch_mode_to(
    WinTCExplorerWindow*    wnd,
    WinTCExplorerWindowMode mode
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

static void on_browser_load_changed(
    WinTCShBrowser*         self,
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
);
static void on_webkit_browser_notify_title(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_webkit_browser_load_changed(
    WebKitWebView*  self,
    WebKitLoadEvent load_event,
    gpointer        user_data
);

//
// STATIC DATA
//
static const gchar* S_TITLE_LOCAL        = "Windows Explorer";
static const gchar* S_TITLE_FORMAT_LOCAL = "%s";

static const gchar* S_TITLE_INTERNET        =
    "Microsoft Internet Explorer";
static const gchar* S_TITLE_FORMAT_INTERNET =
    "%s - Microsoft Internet Explorer";

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
    gchar* initial_path;

    WinTCShextPathInfo local_path;
    gchar*             internet_path;

    WinTCExplorerWindowMode mode;

    WinTCExplorerToolbar* toolbar_adr;
    WinTCExplorerToolbar* toolbar_std;

    // UI
    //
    WinTCShIconViewBehaviour* behaviour_icons;

    GtkWidget* box_toolbars;

    GtkWidget* scrollwnd_main;
    GtkWidget* iconview_browser;
    GtkWidget* webkit_browser;

    GtkWidget* throbber;
    guint      throbber_anim_id;
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
    g_object_class_install_property(
        object_class,
        PROP_INITIAL_PATH,
        g_param_spec_string(
            "initial-path",
            "InitialPath",
            "The initial path for the window to open.",
            NULL,
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
    wintc_explorer_window_signals[SIGNAL_MODE_CHANGED] =
        g_signal_new(
            "mode-changed",
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

    self->box_toolbars =
        GTK_WIDGET(gtk_builder_get_object(builder, "box-toolbars"));
    self->scrollwnd_main =
        GTK_WIDGET(gtk_builder_get_object(builder, "scrollwnd-main"));
    self->throbber =
        GTK_WIDGET(gtk_builder_get_object(builder, "throbber"));

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

    // Attempt to set the throbber pixbuf
    //
    GdkPixbuf* throbber_pixbuf =
        wintc_explorer_application_get_throbber_pixbuf();

    if (throbber_pixbuf)
    {
        self->throbber_anim_id =
            wintc_ctl_animation_add_framesheet(
                WINTC_CTL_ANIMATION(self->throbber),
                throbber_pixbuf,
                50
            );
    }
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

    // Handle initial path
    //
    if (!wnd->initial_path)
    {
        // Nav to desktop if there's no path
        //
        do_navigation(
            wnd,
            wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
        );
    }
    else
    {
        do_navigation(
            wnd,
            wnd->initial_path
        );
    }

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

    g_clear_object(&(wnd->iconview_browser));
    g_clear_object(&(wnd->webkit_browser));

    g_clear_object(&(wnd->toolbar_adr));
    g_clear_object(&(wnd->toolbar_std));

    (G_OBJECT_CLASS(wintc_explorer_window_parent_class))->dispose(object);
}

static void wintc_explorer_window_finalize(
    GObject* object
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(object);

    g_free(wnd->initial_path);
    g_free(wnd->internet_path);
    wintc_shext_path_info_free_data(&(wnd->local_path));

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

        case PROP_INITIAL_PATH:
            wnd->initial_path = g_value_dup_string(value);
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
    WinTCShextHost*           shext_host,
    const gchar*              initial_path
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_EXPLORER_WINDOW,
            "application",  GTK_APPLICATION(app),
            "shext-host",   shext_host,
            "initial-path", initial_path,
            NULL
        )
    );
}

void wintc_explorer_window_get_location(
    WinTCExplorerWindow* wnd,
    WinTCShextPathInfo*  path_info
)
{
    switch (wnd->mode)
    {
        case WINTC_EXPLORER_WINDOW_MODE_LOCAL:
            if (!wnd->local_path.base_path)
            {
                return;
            }

            wintc_shext_path_info_copy(
                path_info,
                &(wnd->local_path)
            );

            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            wintc_shext_path_info_free_data(path_info);

            path_info->base_path = g_strdup(wnd->internet_path);

            break;

        default:
            g_critical("explorer: cannot get location - invalid mode");
            break;
    }
}

WinTCExplorerWindowMode wintc_explorer_window_get_mode(
    WinTCExplorerWindow* wnd
)
{
    return wnd->mode;
}

//
// PRIVATE FUNCTIONS
//
static void do_navigation(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
)
{
    wintc_ctl_animation_play(
        WINTC_CTL_ANIMATION(wnd->throbber),
        wnd->throbber_anim_id,
        0,
        WINTC_CTL_ANIMATION_INFINITE
    );

    // Check out how we should navigate - local or IE?
    //
    // This isn't exactly 'elegant' but I don't really care 'cos it works
    //
    gchar* ipath = g_utf8_strdown(specified_path, -1);

    if (
        g_str_has_prefix(ipath, "http://") ||
        g_str_has_prefix(ipath, "https://")
    )
    {
        switch_mode_to(wnd, WINTC_EXPLORER_WINDOW_MODE_INTERNET);
        do_navigation_internet(wnd, specified_path);
    }
    else
    {
        switch_mode_to(wnd, WINTC_EXPLORER_WINDOW_MODE_LOCAL);
        do_navigation_local(wnd, specified_path);
    }

    g_free(ipath);
}

static void do_navigation_internet(
    WinTCExplorerWindow* wnd,
    const gchar*         specified_path
)
{
    // Probably need to do some other stuff here, for now we just load the URI
    //
    webkit_web_view_load_uri(
        WEBKIT_WEB_VIEW(wnd->webkit_browser),
        specified_path
    );
}

static void do_navigation_local(
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
        &(wnd->local_path)
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
            &(wnd->local_path),
            &path_info
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
    WinTCShextPathInfo* local_path_info
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
        wintc_shext_path_info_free_data(local_path_info);

        local_path_info->base_path = g_strdup(specified_path);

        return;
    }

    // If the path starts with a leading slash '/' then assume an absolute file
    // system path
    //
    if (g_str_has_prefix(specified_path, "/"))
    {
        wintc_shext_path_info_free_data(local_path_info);

        local_path_info->base_path =
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
        wintc_shext_path_info_free_data(local_path_info);

        local_path_info->base_path = g_strdup(specified_path);

        return;
    }

    // Here we assume it's a relative path
    // FIXME: When HTTP(S) is supported, we will need to check the current path
    //
    g_free(local_path_info->extended_path);

    local_path_info->extended_path = g_strdup(specified_path);
}

static void switch_mode_to(
    WinTCExplorerWindow*    wnd,
    WinTCExplorerWindowMode mode
)
{
    if (mode == wnd->mode)
    {
        return;
    }

    // Close existing mode
    //
    switch (wnd->mode)
    {
        case WINTC_EXPLORER_WINDOW_MODE_LOCAL:
            if (wnd->iconview_browser)
            {
                gtk_container_remove(
                    GTK_CONTAINER(wnd->scrollwnd_main),
                    wnd->iconview_browser
                );
            }

            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            if (wnd->webkit_browser)
            {
                gtk_container_remove(
                    GTK_CONTAINER(wnd->scrollwnd_main),
                    wnd->webkit_browser
                );
            }

            break;

        default: break;
    }

    // Open new mode
    //
    gchar* wnd_title = NULL;

    wnd->mode = mode;

    switch (wnd->mode)
    {
        case WINTC_EXPLORER_WINDOW_MODE_LOCAL:
            if (!wnd->iconview_browser)
            {
                wnd->iconview_browser = gtk_icon_view_new();

                g_object_ref(wnd->iconview_browser);

                // Set up shell browser
                //
                wnd->browser = wintc_sh_browser_new(wnd->shext_host);

                g_signal_connect(
                    wnd->browser,
                    "load-changed",
                    G_CALLBACK(on_browser_load_changed),
                    wnd
                );

                // Link up with UI
                //
                wnd->behaviour_icons =
                    wintc_sh_icon_view_behaviour_new(
                        GTK_ICON_VIEW(wnd->iconview_browser),
                        wnd->browser
                    );
            }

            // Update view
            //
            wnd_title = g_strdup_printf("%s", S_TITLE_LOCAL);

            gtk_window_set_icon_name(
                GTK_WINDOW(wnd),
                "explorer"
            );

            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_main),
                wnd->iconview_browser
            );

            break;

        case WINTC_EXPLORER_WINDOW_MODE_INTERNET:
            if (!wnd->webkit_browser)
            {
                wnd->webkit_browser = webkit_web_view_new();

                g_object_ref(wnd->webkit_browser);

                // Connect signals
                //
                g_signal_connect(
                    wnd->webkit_browser,
                    "load-changed",
                    G_CALLBACK(on_webkit_browser_load_changed),
                    wnd
                );
                g_signal_connect(
                    wnd->webkit_browser,
                    "notify::title",
                    G_CALLBACK(on_webkit_browser_notify_title),
                    wnd
                );

                gtk_widget_show(wnd->webkit_browser);
            }

            // Update view
            //
            wnd_title = g_strdup_printf("%s", S_TITLE_INTERNET);

            gtk_window_set_icon_name(
                GTK_WINDOW(wnd),
                "text-html"
            );

            gtk_container_add(
                GTK_CONTAINER(wnd->scrollwnd_main),
                wnd->webkit_browser
            );

            break;

        default:
            g_critical("explorer: unknown window mode %d", mode);
            break;
    }

    gtk_window_set_title(
        GTK_WINDOW(wnd),
        wnd_title
    );

    g_free(wnd_title);

    // Emit signal for things like toolbars etc.
    //
    g_signal_emit(
        wnd,
        wintc_explorer_window_signals[SIGNAL_MODE_CHANGED],
        0
    );
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

static void on_browser_load_changed(
    WinTCShBrowser*         self,
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    ca_context* ctx;
    gchar*      wnd_title = NULL;

    WINTC_LOG_DEBUG("explorer: shell browser load changed to %d", load_event);

    switch (load_event)
    {
        case WINTC_SH_BROWSER_LOAD_STARTED:
            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(wnd->throbber),
                wnd->throbber_anim_id,
                10,
                WINTC_CTL_ANIMATION_INFINITE
            );

            // Play the lovely navigation sound
            //
            ctx = ca_gtk_context_get();

            ca_context_play(
                ctx,
                0,
                CA_PROP_EVENT_ID, "browser-navigate",
                NULL
            );

            // Update our local state and emit on the window
            //
            wintc_shext_path_info_free_data(&(wnd->local_path));

            wintc_sh_browser_get_location(
                wnd->browser,
                &(wnd->local_path)
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

            break;

        case WINTC_SH_BROWSER_LOAD_FINISHED:
            wnd_title =
                g_strdup_printf(
                    S_TITLE_FORMAT_LOCAL,
                    wintc_sh_browser_get_view_display_name(self)
                );

            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(wnd->throbber),
                wnd->throbber_anim_id,
                0,
                WINTC_CTL_ANIMATION_INFINITE
            );

            break;
    }

    if (wnd_title)
    {
        gtk_window_set_title(
            GTK_WINDOW(wnd),
            wnd_title
        );

        g_free(wnd_title);
    }
}

static void on_webkit_browser_notify_title(
    WINTC_UNUSED(GObject*   self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    if (wnd->mode != WINTC_EXPLORER_WINDOW_MODE_INTERNET)
    {
        return;
    }

    // Just set the new title
    //
    const gchar* title =
        webkit_web_view_get_title(
            WEBKIT_WEB_VIEW(wnd->webkit_browser)
        );

    if (!title)
    {
        return;
    }

    gchar* wnd_title =
        g_strdup_printf(
            S_TITLE_FORMAT_INTERNET,
            title
        );

    gtk_window_set_title(
        GTK_WINDOW(wnd),
        wnd_title
    );

    g_free(wnd_title);
}

static void on_webkit_browser_load_changed(
    WINTC_UNUSED(WebKitWebView* self),
    WebKitLoadEvent load_event,
    gpointer        user_data
)
{
    WinTCExplorerWindow* wnd = WINTC_EXPLORER_WINDOW(user_data);

    // Throbber logic...
    //
    switch (load_event)
    {
        case WEBKIT_LOAD_STARTED:
            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(wnd->throbber),
                wnd->throbber_anim_id,
                10,
                WINTC_CTL_ANIMATION_INFINITE
            );

            break;

        case WEBKIT_LOAD_FINISHED:
            wintc_ctl_animation_play(
                WINTC_CTL_ANIMATION(wnd->throbber),
                wnd->throbber_anim_id,
                0,
                WINTC_CTL_ANIMATION_INFINITE
            );

            break;

        default: break;
    }

    // Location logic
    //
    ca_context* ctx;

    switch (load_event)
    {
        case WEBKIT_LOAD_STARTED:
        case WEBKIT_LOAD_REDIRECTED:
            ctx = ca_gtk_context_get();

            ca_context_play(
                ctx,
                0,
                CA_PROP_EVENT_ID, "browser-navigate",
                NULL
            );

            // Fall-through...

        case WEBKIT_LOAD_COMMITTED:
            wintc_strdup_replace(
                &(wnd->internet_path),
                webkit_web_view_get_uri(
                    WEBKIT_WEB_VIEW(wnd->webkit_browser)
                )
            );

            g_signal_emit(
                wnd,
                wintc_explorer_window_signals[SIGNAL_LOCATION_CHANGED],
                0
            );

            break;

        default: break;
    }
}
