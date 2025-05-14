#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskmgrApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCTaskmgrApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_taskmgr_application_activate(
    GApplication* application
);
static void wintc_taskmgr_application_startup(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCTaskmgrApplication,
    wintc_taskmgr_application,
    GTK_TYPE_APPLICATION
)

static void wintc_taskmgr_application_class_init(
    WinTCTaskmgrApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_taskmgr_application_activate;
    application_class->startup  = wintc_taskmgr_application_startup;
}

static void wintc_taskmgr_application_init(
    WINTC_UNUSED(WinTCTaskmgrApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_taskmgr_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_taskmgr_window_new(WINTC_TASKMGR_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

static void wintc_taskmgr_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_taskmgr_application_parent_class))
        ->startup(application);

    wintc_ctl_install_default_styles();
}

//
// PUBLIC FUNCTIONS
//
WinTCTaskmgrApplication* wintc_taskmgr_application_new(void)
{
    return WINTC_TASKMGR_APPLICATION(
        g_object_new(
            wintc_taskmgr_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.samples.taskmgr",
            NULL
        )
    );
}
