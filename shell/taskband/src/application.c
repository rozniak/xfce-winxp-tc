#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "dispproto.h"
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
static void wintc_taskband_application_startup(
    GApplication* application
);

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

    application_class->activate = wintc_taskband_application_activate;
    application_class->startup  = wintc_taskband_application_startup;
}

static void wintc_taskband_application_init(
    WINTC_UNUSED(WinTCTaskbandApplication* self)
) {}

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
    if (!init_display_protocol_apis())
    {
        g_critical("%s", "Failed to resolve display protocol APIs.");
        g_application_quit(application);
    }
}
