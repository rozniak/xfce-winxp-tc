#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>

#include "application.h"
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
    WinTCShextHost* shext_host;
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_application_activate(
    GApplication* application
);
static void wintc_explorer_application_startup(
    GApplication* application
);

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

    application_class->activate = wintc_explorer_application_activate;
    application_class->startup  = wintc_explorer_application_startup;
}

static void wintc_explorer_application_init(
    WINTC_UNUSED(WinTCExplorerApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_explorer_application_activate(
    GApplication* application
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(application);

    GtkWidget* new_window =
        wintc_explorer_window_new(
            explorer_app,
            explorer_app->shext_host
        );

    gtk_widget_show_all(new_window);
}

static void wintc_explorer_application_startup(
    GApplication* application
)
{
    WinTCExplorerApplication* explorer_app =
        WINTC_EXPLORER_APPLICATION(application);

    (G_APPLICATION_CLASS(wintc_explorer_application_parent_class))
        ->startup(application);

    // Init comctl
    //
    wintc_ctl_install_default_styles();

    // Create shext host instance
    //
    explorer_app->shext_host = wintc_shext_host_new();

    wintc_sh_init_builtin_extensions(explorer_app->shext_host);
    wintc_shext_host_load_extensions(
        explorer_app->shext_host,
        WINTC_SHEXT_LOAD_DEFAULT,
        NULL
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerApplication* wintc_explorer_application_new(void)
{
    return WINTC_EXPLORER_APPLICATION(
        g_object_new(
            wintc_explorer_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.explorer",
            NULL
        )
    );
}
