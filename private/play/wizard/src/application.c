#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWizardApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCWizardApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_wizard_application_activate(
    GApplication* application
);
static void wintc_wizard_application_startup(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWizardApplication,
    wintc_wizard_application,
    GTK_TYPE_APPLICATION
)

static void wintc_wizard_application_class_init(
    WinTCWizardApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_wizard_application_activate;
    application_class->startup  = wintc_wizard_application_startup;
}

static void wintc_wizard_application_init(
    WINTC_UNUSED(WinTCWizardApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_wizard_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_wizard_window_new(WINTC_WIZARD_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

static void wintc_wizard_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_wizard_application_parent_class))
        ->startup(application);

    wintc_ctl_install_default_styles();
}

//
// PUBLIC FUNCTIONS
//
WinTCWizardApplication* wintc_wizard_application_new(void)
{
    return WINTC_WIZARD_APPLICATION(
        g_object_new(
            wintc_wizard_application_get_type(),
            "application-id", "uk.oddmatics.wintc.play.wizard",
            NULL
        )
    );
}
