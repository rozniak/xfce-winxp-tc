#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comctl.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// FORWARD DECLARATIONS
//
static void wintc_resvwr_application_activate(
    GApplication* application
);
static void wintc_resvwr_application_startup(
    GApplication* application
);

//
// STATIC DATA
//
static const WinTCAccelEntry S_ACCELS[] = {
    {
        "win.open",
        {
            "<Ctrl>O",
            NULL
        }
    },
    {
        "win.refresh",
        {
            "F5",
            NULL
        }
    },
    {
        "win.exit",
        {
            "<Alt>F4",
            NULL
        }
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCResvwrApplication
{
    GtkApplication __parent__;
} WinTCResvwrApplication;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCResvwrApplication,
    wintc_resvwr_application,
    GTK_TYPE_APPLICATION
)

static void wintc_resvwr_application_class_init(
    WinTCResvwrApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_resvwr_application_activate;
    application_class->startup  = wintc_resvwr_application_startup;
}

static void wintc_resvwr_application_init(
    WINTC_UNUSED(WinTCResvwrApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_resvwr_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_resvwr_window_new(WINTC_RESVWR_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

static void wintc_resvwr_application_startup(
    GApplication* application
)
{
    (G_APPLICATION_CLASS(wintc_resvwr_application_parent_class))
        ->startup(application);

    // Init comctl
    //
    wintc_ctl_install_default_styles();

    g_type_ensure(WINTC_TYPE_CTL_ANIMATION);

    // Init accelerators
    //
    wintc_application_set_accelerators(
        GTK_APPLICATION(application),
        S_ACCELS,
        G_N_ELEMENTS(S_ACCELS)
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCResvwrApplication* wintc_resvwr_application_new(void)
{
    return WINTC_RESVWR_APPLICATION(
        g_object_new(
            wintc_resvwr_application_get_type(),
            "application-id", "uk.oddmatics.wintc.tools.resvwr",
            NULL
        )
    );
}
