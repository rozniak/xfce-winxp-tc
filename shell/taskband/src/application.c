#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskbandApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCTaskbandApplication
{
    GtkApplication __parent__;

    GtkWidget* host_window;
};

//
// FORWARD DECLARATIONS
//
static void wintc_taskband_application_activate(
    GApplication* application
);
static gint wintc_taskband_application_command_line(
    GApplication*            application,
    GApplicationCommandLine* command_line
);
static gint wintc_taskband_application_handle_local_options(
    GApplication* application,
    GVariantDict* options
);
static void wintc_taskband_application_shutdown(
    GApplication* application
);
static void wintc_taskband_application_startup(
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
        "Quit a running Windows taskband instance.",
        NULL
    },
    { 0 }
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCTaskbandApplication,
    wintc_taskband_application,
    GTK_TYPE_APPLICATION
)

static void wintc_taskband_application_class_init(
    WinTCTaskbandApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate =
        wintc_taskband_application_activate;
    application_class->command_line =
        wintc_taskband_application_command_line;
    application_class->handle_local_options =
        wintc_taskband_application_handle_local_options;
    application_class->shutdown =
        wintc_taskband_application_shutdown;
    application_class->startup =
        wintc_taskband_application_startup;
}

static void wintc_taskband_application_init(
    WinTCTaskbandApplication* self
)
{
    g_application_add_main_option_entries(
        G_APPLICATION(self),
        S_OPTIONS
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCTaskbandApplication* wintc_taskband_application_new(void)
{
    WinTCTaskbandApplication* app;

    g_set_application_name("Taskband");

    app =
        g_object_new(
            wintc_taskband_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.taskband",
            "flags",          G_APPLICATION_HANDLES_COMMAND_LINE,
            NULL
        );

    return app;
}

//
// CALLBACKS
//
static void wintc_taskband_application_activate(
    GApplication* application
)
{
    WinTCTaskbandApplication* taskband_app =
        WINTC_TASKBAND_APPLICATION(application);

    if (taskband_app->host_window != NULL)
    {
        return;
    }

    taskband_app->host_window = wintc_taskband_window_new(taskband_app);

    gtk_widget_show_all(taskband_app->host_window);
}

static gint wintc_taskband_application_command_line(
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

static gint wintc_taskband_application_handle_local_options(
    WINTC_UNUSED(GApplication* application),
    WINTC_UNUSED(GVariantDict* options)
)
{
    // Stub
    return -1;
}

static void wintc_taskband_application_shutdown(
    GApplication* application
)
{
    GList* window_iter = NULL;

    while (
        (window_iter =
            gtk_application_get_windows(GTK_APPLICATION(application)))
    )
    {
        gtk_widget_destroy(GTK_WIDGET(window_iter->data));
    }

    (G_APPLICATION_CLASS(wintc_taskband_application_parent_class))
        ->shutdown(application);
}

static void wintc_taskband_application_startup(
    GApplication* application
)
{
    // Chain up for gtk init
    //
    G_APPLICATION_CLASS(wintc_taskband_application_parent_class)
        ->startup(application);

    // Init APIs at runtime
    //
    if (!wintc_init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        g_application_quit(application);
    }
}
