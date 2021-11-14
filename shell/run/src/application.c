#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "dialog.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCRunApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCRunApplication
{
    GtkApplication __parent__;

    GtkWidget* main_window;
};

//
// FORWARD DECLARATIONS
//
static void wintc_run_application_activate(
    GApplication* application
);

static void wintc_run_application_finalize(
    GObject* object
);

static void wintc_run_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    const gchar*   hint
);

static void wintc_run_application_startup(
    GApplication* application
);

static void wintc_run_application_shutdown(
    GApplication* application
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(WinTCRunApplication, wintc_run_application, GTK_TYPE_APPLICATION);

static void wintc_run_application_class_init(
    WinTCRunApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);
    GObjectClass*      object_class      = G_OBJECT_CLASS(klass);

    application_class->activate = wintc_run_application_activate;
    application_class->open     = wintc_run_application_open;
    application_class->startup  = wintc_run_application_startup;
    application_class->shutdown = wintc_run_application_shutdown;

    object_class->finalize = wintc_run_application_finalize;
}

static void wintc_run_application_init(
    WinTCRunApplication* self
) {}

//
// FINALIZE
//
static void wintc_run_application_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(wintc_run_application_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
WinTCRunApplication* wintc_run_application_new(void)
{
    WinTCRunApplication* app;

    g_set_application_name("Run");

    app =
        g_object_new(
            wintc_run_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.run",
            NULL
        );

    return app;
}

//
// CALLBACKS
//
static void wintc_run_application_activate(
    GApplication* application
)
{
    WinTCRunApplication* run_app = WINTC_RUN_APPLICATION(application);

    wintc_focus_window(GTK_WINDOW(run_app->main_window));
}

static void wintc_run_application_open(
    GApplication* application,
    GFile**       files,
    int           n_files,
    const gchar*  hint
)
{
}

static void wintc_run_application_startup(
    GApplication* application
)
{
    WinTCRunApplication* run_app = WINTC_RUN_APPLICATION(application);

    (G_APPLICATION_CLASS(wintc_run_application_parent_class))->startup(application);

    run_app->main_window = wintc_run_dialog_new(run_app);

    gtk_widget_show_all(run_app->main_window);
}

static void wintc_run_application_shutdown(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_run_application_parent_class))->shutdown(application);
}
