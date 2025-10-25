#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
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

    WinTCDesktopSettings* settings;
    WinTCShextHost*       shext_host;
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

    g_clear_object(&(desktop_app->shext_host));

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

    // FIXME: Just create a desktop window per monitor for now, connect up
    //        signals for monitors added/removed later
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
                                  gdk_monitor_is_primary(monitor) ?
                                      desktop_app->shext_host : NULL
                              );

        gtk_widget_show_all(wnd);
    }
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
    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(application);

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

    // Init shext host
    //
    desktop_app->shext_host = wintc_shext_host_new();

    wintc_sh_init_builtin_extensions(desktop_app->shext_host);
    wintc_shext_host_load_extensions(
        desktop_app->shext_host,
        WINTC_SHEXT_LOAD_DEFAULT,
        NULL
    );
}

//
// PUBLIC MEHTODS
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
