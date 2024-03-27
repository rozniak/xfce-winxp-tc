#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// CONSTANTS
//
static const gchar* S_APP_ID = "uk.co.oddmatics.wintc.cpl-sysdm";

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplSysdmApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCCplSysdmApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_sysdm_application_activate(
    GApplication* application
);
static void wintc_cpl_sysdm_application_startup(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCplSysdmApplication,
    wintc_cpl_sysdm_application,
    GTK_TYPE_APPLICATION
)

static void wintc_cpl_sysdm_application_class_init(
    WinTCCplSysdmApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_cpl_sysdm_application_activate;
    application_class->startup  = wintc_cpl_sysdm_application_startup;
}

static void wintc_cpl_sysdm_application_init(
    WINTC_UNUSED(WinTCCplSysdmApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_sysdm_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_cpl_sysdm_window_new(WINTC_CPL_SYSDM_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

static void wintc_cpl_sysdm_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_cpl_sysdm_application_parent_class))
        ->startup(application);

    g_set_prgname(S_APP_ID);

    wintc_ctl_install_default_styles();
}

//
// PUBLIC FUNCTIONS
//
WinTCCplSysdmApplication* wintc_cpl_sysdm_application_new(void)
{
    return WINTC_CPL_SYSDM_APPLICATION(
        g_object_new(
            wintc_cpl_sysdm_application_get_type(),
            "application-id", S_APP_ID,
            NULL
        )
    );
}
