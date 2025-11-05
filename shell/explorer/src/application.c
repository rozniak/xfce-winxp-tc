#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>

#include "application.h"
#include "loader.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExplorerApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCExplorerApplication
{
    GtkApplication __parent__;

    // Application state
    //
    WinTCExplorerLoader*  exp_loader;
    WinTCShFolderOptions* fldr_opts;
    WinTCShextHost*       shext_host;
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_application_dispose(
    GObject* object
);

static void wintc_explorer_application_activate(
    GApplication* application
);
static gint wintc_explorer_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
);
static gint wintc_explorer_application_handle_local_options(
    GApplication* application,
    GVariantDict* options
);
static void wintc_explorer_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    const gchar*  hint
);
static void wintc_explorer_application_startup(
    GApplication* application
);

//
// STATIC DATA
//
static const GOptionEntry S_OPTIONS[] = {
    {
        "ie",
        '\0',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_NONE,
        NULL,
        "Launch an IE window, if no URLs specified.",
        NULL
    },
    {
        G_OPTION_REMAINING,
        '\0',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_FILENAME_ARRAY,
        NULL,
        NULL,
        "[URLS...]"
    },
    { 0 }
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCExplorerApplication,
    wintc_explorer_application,
    GTK_TYPE_APPLICATION
)

static void wintc_explorer_application_class_init(
    WinTCExplorerApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);
    GObjectClass*      object_class      = G_OBJECT_CLASS(klass);

    application_class->activate =
        wintc_explorer_application_activate;
    application_class->command_line =
        wintc_explorer_application_command_line;
    application_class->handle_local_options =
        wintc_explorer_application_handle_local_options;
    application_class->open =
        wintc_explorer_application_open;
    application_class->startup =
        wintc_explorer_application_startup;

    object_class->dispose = wintc_explorer_application_dispose;
}

static void wintc_explorer_application_init(
    WinTCExplorerApplication* self
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
static void wintc_explorer_application_dispose(
    GObject* object
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(object);

    g_clear_object(&(explorer_app->shext_host));
    g_clear_object(&(explorer_app->fldr_opts));
    g_clear_object(&(explorer_app->exp_loader));

    (G_OBJECT_CLASS(wintc_explorer_application_parent_class))
        ->dispose(object);
}

static void wintc_explorer_application_activate(
    GApplication* application
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(application);

    GtkWidget* new_window =
        wintc_explorer_window_new(
            explorer_app,
            explorer_app->shext_host,
            explorer_app->fldr_opts,
            explorer_app->exp_loader,
            NULL
        );

    gtk_widget_show_all(new_window);
}

static gint wintc_explorer_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
)
{
    GVariantDict* options =
        g_application_command_line_get_options_dict(command_line);

    gboolean hint_ie = FALSE;

    // Check for IE hint
    //
    if (g_variant_dict_contains(options, "ie"))
    {
        hint_ie = TRUE;
    }

    // Handle URIs passed in
    //
    guint   n_uris = 0;
    GFile** uris   = wintc_application_command_line_get_files(
                         command_line,
                         &n_uris
                     );
    if (uris)
    {
        g_application_open(application, uris, n_uris, NULL);

        wintc_freenv(uris, n_uris, g_object_unref);
    }
    else
    {
        // No URIs - open either default explorer window, or IE home page
        //
        if (hint_ie)
        {
            // FIXME: Defaulting to msn.com because we have no home page
            //        setting yet
            //
            GFile* home_page =
                g_file_new_for_uri("https://www.msn.com");

            g_application_open(application, &home_page, 1, NULL);

            g_object_unref(home_page);
        }
        else
        {
            g_application_activate(application);
        }
    }

    return 0;
}

static gint wintc_explorer_application_handle_local_options(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GVariantDict* options)
)
{
    // Stub
    return -1;
}

static void wintc_explorer_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    WINTC_UNUSED(const gchar* hint)
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(application);

    for (int i = 0; i < n_files; i++)
    {
        GFile*     file = files[i];
        gchar*     uri  = g_file_get_uri(file);
        GtkWidget* wnd  = wintc_explorer_window_new(
                              explorer_app,
                              explorer_app->shext_host,
                              explorer_app->fldr_opts,
                              explorer_app->exp_loader,
                              uri
                          );

        gtk_widget_show_all(wnd);

        g_free(uri);
    }
}

static void wintc_explorer_application_startup(
    GApplication* application
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(application);

    (G_APPLICATION_CLASS(wintc_explorer_application_parent_class))
        ->startup(application);

    // Install styles
    //
    GtkCssProvider* css_provider   = gtk_css_provider_new();
    GtkCssProvider* css_provider_p = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_provider,
        "/uk/oddmatics/wintc/explorer/appstyles.css"
    );
    gtk_css_provider_load_from_resource(
        css_provider_p,
        "/uk/oddmatics/wintc/explorer/appstyles_p.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider_p),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Install icon resource path
    //
    gtk_icon_theme_add_resource_path(
        gtk_icon_theme_get_default(),
        "/uk/oddmatics/wintc/explorer"
    );

    // Init comctl
    //
    wintc_ctl_install_default_styles();

    // Init sidebars/toolbars
    //
    explorer_app->exp_loader = wintc_explorer_loader_new();

    wintc_explorer_loader_load_extensions(
        explorer_app->exp_loader
    );

    // Create shext host instance
    //
    explorer_app->shext_host = wintc_shext_host_new();

    wintc_sh_init_builtin_extensions(explorer_app->shext_host);
    wintc_shext_host_load_extensions(
        explorer_app->shext_host,
        WINTC_SHEXT_LOAD_DEFAULT,
        NULL
    );

    // Create folder options
    //
    explorer_app->fldr_opts = wintc_sh_folder_options_new();
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerApplication* wintc_explorer_application_new(void)
{
    const GApplicationFlags k_app_flags =
        G_APPLICATION_HANDLES_OPEN |
        G_APPLICATION_HANDLES_COMMAND_LINE;

    return WINTC_EXPLORER_APPLICATION(
        g_object_new(
            wintc_explorer_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.explorer",
            "flags",          k_app_flags,
            NULL
        )
    );
}

GdkPixbuf* wintc_explorer_application_get_throbber_pixbuf(void)
{
    static GdkPixbuf* pixbuf = NULL;

    if (!pixbuf)
    {
        GError* error = NULL;

        pixbuf =
            gdk_pixbuf_new_from_resource(
                "/uk/oddmatics/wintc/explorer/flag22.png",
                &error
            );

        wintc_log_error_and_clear(&error);
    }

    return pixbuf;
}
