#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "lnkwiz.h"

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_appwiz_application_activate(
    GApplication* application
);
static void wintc_cpl_appwiz_application_startup(
    GApplication* application
);

static void wintc_cpl_appwiz_application_new_link_here(
    const gchar* path
);

//
// STATIC DATA
//
static gchar* S_ARG_PATH_NEW_LINK = NULL;

static const GOptionEntry S_OPTION_ENTRIES[] = {
    {
        "new-link-here",
        0,
        G_OPTION_FLAG_NONE,
        G_OPTION_ARG_FILENAME,
        &S_ARG_PATH_NEW_LINK,
        "Spawn the new link wizard.",
        NULL
    },
    { 0 }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplAppwizApplication
{
    GtkApplication __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCplAppwizApplication,
    wintc_cpl_appwiz_application,
    GTK_TYPE_APPLICATION
)

static void wintc_cpl_appwiz_application_class_init(
    WinTCCplAppwizApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_cpl_appwiz_application_activate;
    application_class->startup  = wintc_cpl_appwiz_application_startup;
}

static void wintc_cpl_appwiz_application_init(
    WinTCCplAppwizApplication* self
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
static void wintc_cpl_appwiz_application_activate(
    WINTC_UNUSED(GApplication* application)
)
{
    if (S_ARG_PATH_NEW_LINK)
    {
        wintc_cpl_appwiz_application_new_link_here(
            S_ARG_PATH_NEW_LINK
        );

        goto cleanup;
    }

cleanup:
    g_free(S_ARG_PATH_NEW_LINK);
}

static void wintc_cpl_appwiz_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_cpl_appwiz_application_parent_class))
        ->startup(application);

    wintc_ctl_install_default_styles();
}

//
// PUBLIC FUNCTIONS
//
WinTCCplAppwizApplication* wintc_cpl_appwiz_application_new(void)
{
    WinTCCplAppwizApplication* app;

    app =
        g_object_new(
            wintc_cpl_appwiz_application_get_type(),
            "application-id", "uk.oddmatics.wintc.cpl-appwiz",
            NULL
        );

    return app;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_cpl_appwiz_application_new_link_here(
    const gchar* path
)
{
    GtkWidget* wizard =
        wintc_cpl_appwiz_new_link_wizard_new(path);

    gtk_window_set_application(
        GTK_WINDOW(wizard),
        GTK_APPLICATION(g_application_get_default())
    );

    gtk_widget_show_all(wizard);
}
