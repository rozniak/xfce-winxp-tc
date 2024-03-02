#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSANCE DEFINITIONS
//
struct _WinTCDesktopApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCDesktopApplication
{
    GtkApplication __parent__;

    GtkWidget* host_window;
};

//
// FORWARD DECLARATIONS
//
static void wintc_desktop_application_activate(
    GApplication* application
);
static void wintc_desktop_application_startup(
    GApplication* application
);

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

    application_class->activate = wintc_desktop_application_activate;
    application_class->startup  = wintc_desktop_application_startup;
}

static void wintc_desktop_application_init(
    WINTC_UNUSED(WinTCDesktopApplication* self)
) {}

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
            NULL
        );

    return app;
}

//
// CALLBACKS
//
static void wintc_desktop_application_activate(
    GApplication* application
)
{
    WinTCDesktopApplication* desktop_app =
        WINTC_DESKTOP_APPLICATION(application);

    if (desktop_app->host_window != NULL)
    {
        return;
    }

    desktop_app->host_window = wintc_desktop_window_new(desktop_app);

    gtk_widget_show_all(desktop_app->host_window);
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
}
