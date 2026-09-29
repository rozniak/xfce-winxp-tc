#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "inswiz.h"

//
// FORWARD DECLARATIONS
//
static void wintc_pkg_exec_application_activate(
    GApplication* application
);
static void wintc_pkg_exec_application_startup(
    GApplication* application
);

static void wintc_pkg_exec_application_install_wizard(
    const gchar* path
);

//
// STATIC DATA
//
static gchar* S_ARG_PATH_INSTALL = NULL;

static const GOptionEntry S_OPTION_ENTRIES[] = {
    {
        "install",
        'i',
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_FILENAME,
        &S_ARG_PATH_INSTALL,
        "Install a package using the installation wizard.",
        NULL
    },
    { 0 }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCPkgExecApplication
{
    GtkApplication __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCPkgExecApplication,
    wintc_pkg_exec_application,
    GTK_TYPE_APPLICATION
)

static void wintc_pkg_exec_application_class_init(
    WinTCPkgExecApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_pkg_exec_application_activate;
    application_class->startup  = wintc_pkg_exec_application_startup;
}

static void wintc_pkg_exec_application_init(
    WinTCPkgExecApplication* self
)
{
    g_application_add_main_option_entries(
        G_APPLICATION(self),
        S_OPTION_ENTRIES
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_pkg_exec_application_activate(
    WINTC_UNUSED(GApplication* application)
)
{
    if (S_ARG_PATH_INSTALL)
    {
        gchar* pkg_path =
            g_canonicalize_filename(S_ARG_PATH_INSTALL, NULL);

        wintc_pkg_exec_application_install_wizard(pkg_path);

        g_free(pkg_path);

        goto cleanup;
    }

cleanup:
    g_free(S_ARG_PATH_INSTALL);
}

static void wintc_pkg_exec_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_pkg_exec_application_parent_class))
        ->startup(application);

    wintc_ctl_install_default_styles();
}

//
// PUBLIC FUNCTIONS
//
WinTCPkgExecApplication* wintc_pkg_exec_application_new(void)
{
    WinTCPkgExecApplication* app;

    app =
        g_object_new(
            wintc_pkg_exec_application_get_type(),
            "application-id", "uk.oddmatics.wintc.pkg-exec",
            NULL
        );

    return app;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_pkg_exec_application_install_wizard(
    const gchar* path
)
{
    GtkWidget* wizard =
        wintc_pkg_exec_install_wizard_new(path);

    gtk_window_set_application(
        GTK_WINDOW(wizard),
        GTK_APPLICATION(g_application_get_default())
    );

    gtk_widget_show_all(wizard);
}
