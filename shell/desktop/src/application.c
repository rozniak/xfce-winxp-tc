#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>

#include "application.h"
#include "settings.h"
#include "window.h"

//
// GTK OOP CLASS/INSANCE DEFINITIONS
//
struct _WinTCDesktopApplication
{
    GtkApplication __parent__;

    // State
    //
    WinTCDesktopSettings* settings;

    GList* list_desktops;

    // Shell stuff
    //
    WinTCShextHost*       shext_host;
    WinTCShFolderOptions* fldr_opts;
    WinTCShBrowser*       browser;
};

//
// FORWARD DECLARATIONS
//
static void wintc_desktop_application_dispose(
    GObject* object
);

static void wintc_desktop_application_activate(
    GApplication* application
);
static gint wintc_desktop_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
);
static gint wintc_desktop_application_handle_local_options(
    GApplication* application,
    GVariantDict* options
);
static void wintc_desktop_application_startup(
    GApplication* application
);

static gboolean is_monitor_primary(
    GdkMonitor* monitor
);

static void on_display_monitor_added(
    GdkDisplay* display,
    GdkMonitor* monitor,
    gpointer    user_data
);
static void on_display_monitor_removed(
    GdkDisplay* display,
    GdkMonitor* monitor,
    gpointer    user_data
);

//
// STATIC DATA
//
static const GOptionEntry S_OPTIONS[] = {
    {
        "quit",
        'q',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_NONE,
        NULL,
        "Quit a running Windows desktop instance.",
        NULL
    },
    { 0 }
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCDesktopApplication,
    wintc_desktop_application,
    GTK_TYPE_APPLICATION
)

static void wintc_desktop_application_class_init(
    WinTCDesktopApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);
    GObjectClass*      object_class      = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_desktop_application_dispose;

    application_class->activate =
        wintc_desktop_application_activate;
    application_class->command_line =
        wintc_desktop_application_command_line;
    application_class->handle_local_options =
        wintc_desktop_application_handle_local_options;
    application_class->startup =
        wintc_desktop_application_startup;
}

static void wintc_desktop_application_init(
    WinTCDesktopApplication* self
)
{
    g_application_add_main_option_entries(
        G_APPLICATION(self),
        S_OPTIONS
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_desktop_application_dispose(
    GObject* object
)
{
    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(object);

    g_clear_object(&(desktop_app->browser));
    g_clear_object(&(desktop_app->fldr_opts));
    g_clear_object(&(desktop_app->shext_host));
    g_clear_list(&(desktop_app->list_desktops), NULL);

    (G_OBJECT_CLASS(wintc_desktop_application_parent_class))
        ->dispose(object);
}

static void wintc_desktop_application_activate(
    GApplication* application
)
{
    static gboolean launched = FALSE;

    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(application);

    if (launched)
    {
        return;
    }

    launched = TRUE;

    desktop_app->settings = wintc_desktop_settings_new();

    // Create the shell stuff - the browser will be handed to all, but only
    // used by the current primary desktop
    //
    desktop_app->shext_host = wintc_shext_host_new();

    wintc_sh_init_builtin_extensions(desktop_app->shext_host);
    wintc_shext_host_load_extensions(
        desktop_app->shext_host,
        WINTC_SHEXT_LOAD_DEFAULT,
        NULL
    );

    desktop_app->fldr_opts = wintc_sh_folder_options_new();

    wintc_sh_folder_options_set_browse_in_same_window(
        desktop_app->fldr_opts,
        FALSE
    );

    desktop_app->browser =
        wintc_sh_browser_new(
            desktop_app->shext_host,
            desktop_app->fldr_opts
        );

    wintc_sh_browser_set_behaviour_flags(
        desktop_app->browser,
        WINTC_SH_BROWSER_BEHAVIOUR_DONT_USE_SELF_EXE
    );

    // Navigate to desktop
    //
    WinTCShextPathInfo path_info = { 0 };

    path_info.base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
        );

    wintc_sh_browser_set_location(
        desktop_app->browser,
        &path_info,
        NULL
    );

    wintc_shext_path_info_free_data(&path_info);

    // Set up initial desktop windows
    //
    GdkDisplay* display    = gdk_display_get_default();
    int         n_monitors = gdk_display_get_n_monitors(display);

    for (int i = 0; i < n_monitors; i++)
    {
        GdkMonitor* monitor = gdk_display_get_monitor(display, i);
        GtkWidget*  wnd     = wintc_desktop_window_new(
                                  desktop_app,
                                  monitor,
                                  desktop_app->settings,
                                  desktop_app->browser
                              );

        if (gdk_monitor_is_primary(monitor))
        {
            wintc_desktop_window_set_is_primary(
                WINTC_DESKTOP_WINDOW(wnd),
                TRUE
            );
        }

        gtk_widget_show_all(wnd);

        desktop_app->list_desktops =
            g_list_append(
                desktop_app->list_desktops,
                wnd
            );
    }

    // Connect to display signals to manage monitors
    //
    g_signal_connect(
        display,
        "monitor-added",
        G_CALLBACK(on_display_monitor_added),
        desktop_app
    );
    g_signal_connect(
        display,
        "monitor-removed",
        G_CALLBACK(on_display_monitor_removed),
        desktop_app
    );
}

static gint wintc_desktop_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
)
{
    GVariantDict* options =
        g_application_command_line_get_options_dict(command_line);

    // Just check for --quit
    //
    if (g_variant_dict_contains(options, "quit"))
    {
        g_application_quit(application);
        return 0;
    }

    g_application_activate(application);

    return 0;
}

static gint wintc_desktop_application_handle_local_options(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GVariantDict* options)
)
{
    // Stub
    return -1;
}

static void wintc_desktop_application_startup(
    GApplication* application
)
{
    // Chain up for gtk init
    // 
    G_APPLICATION_CLASS(wintc_desktop_application_parent_class)
        ->startup(application);

    // Init APIs at runtime
    //
    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        g_application_quit(application);
    }

    // Install styles
    //
    GtkCssProvider* css_provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_provider,
        "/uk/oddmatics/wintc/desktop/appstyles_p.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCDesktopApplication* wintc_desktop_application_new(void)
{
    WinTCDesktopApplication* app;

    g_set_application_name("Desktop");

    app =
        g_object_new(
            wintc_desktop_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.desktop",
            "flags",          G_APPLICATION_HANDLES_COMMAND_LINE,
            NULL
        );

    return app;
}

//
// PRIVATE FUNCTIONS
//
static gboolean is_monitor_primary(
    GdkMonitor* monitor
)
{
    if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_WAYLAND)
    {
        return gdk_display_get_monitor(
            gdk_display_get_default(),
            0
        ) == monitor;
    }
    else // X11
    {
        return gdk_monitor_is_primary(monitor);
    }
}

//
// CALLBACKS
//
static void on_display_monitor_added(
    WINTC_UNUSED(GdkDisplay* display),
    GdkMonitor* monitor,
    gpointer    user_data
)
{
    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(user_data);

    gboolean is_primary = is_monitor_primary(monitor);

    // If this is a new primary monitor, check we haven't already got a primary
    // desktop
    //
    if (is_primary)
    {
        for (GList* iter = desktop_app->list_desktops; iter; iter = iter->next)
        {
            WinTCDesktopWindow* wnd = WINTC_DESKTOP_WINDOW(iter->data);

            if (wintc_desktop_window_get_is_primary(wnd))
            {
                wintc_desktop_window_set_is_primary(wnd, FALSE);
                break;
            }
        }
    }

    // Create the new window
    // 
    GtkWidget* wnd =
        wintc_desktop_window_new(
            desktop_app,
            monitor,
            desktop_app->settings,
            desktop_app->browser
        );

    if (is_primary)
    {
        wintc_desktop_window_set_is_primary(
            WINTC_DESKTOP_WINDOW(wnd),
            TRUE
        );
    }

    gtk_widget_show_all(wnd);

    desktop_app->list_desktops =
        g_list_append(
            desktop_app->list_desktops,
            wnd
        );
}

static void on_display_monitor_removed(
    WINTC_UNUSED(GdkDisplay* display),
    GdkMonitor* monitor,
    gpointer    user_data
)
{
    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(user_data);

    // Find the desktop window
    //
    WinTCDesktopWindow* wnd = NULL;

    for (GList* iter = desktop_app->list_desktops; iter; iter = iter->next)
    {
        if (
            wintc_dpa_desktop_window_get_monitor(
                WINTC_DPA_DESKTOP_WINDOW(iter->data)
            ) == monitor
        )
        {
            wnd = WINTC_DESKTOP_WINDOW(iter->data);
            break;
        }
    }

    if (!wnd)
    {
        return;
    }

    // If it was a primary desktop we'll need to select a new one
    //
    if (wintc_desktop_window_get_is_primary(wnd))
    {
        for (GList* iter = desktop_app->list_desktops; iter; iter = iter->next)
        {
            if (
                is_monitor_primary(
                    wintc_dpa_desktop_window_get_monitor(
                        WINTC_DPA_DESKTOP_WINDOW(iter->data)
                    )
                )
            )
            {
                wintc_desktop_window_set_is_primary(
                    WINTC_DESKTOP_WINDOW(iter->data),
                    TRUE
                );
                break;
            }
        }
    }

    // Can now safely destroy the window in peace
    //
    gtk_widget_destroy(GTK_WIDGET(wnd));

    desktop_app->list_desktops =
        g_list_delete_link(
            desktop_app->list_desktops,
            g_list_find(desktop_app->list_desktops, wnd)
        );
}
