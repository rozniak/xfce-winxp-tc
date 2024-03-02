#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCPaTestApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCPaTestApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_patest_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCPaTestApplication,
    wintc_patest_application,
    GTK_TYPE_APPLICATION
)

static void wintc_patest_application_class_init(
    WinTCPaTestApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_patest_application_activate;
}

static void wintc_patest_application_init(
    WINTC_UNUSED(WinTCPaTestApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_patest_application_activate(
    GApplication* application
)
{
    static GtkWidget* the_window = NULL;

    if (the_window)
    {
        wintc_focus_window(GTK_WINDOW(the_window));
    }
    else
    {
        the_window = 
            wintc_patest_window_new(WINTC_PATEST_APPLICATION(application));

        gtk_widget_show_all(the_window);
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCPaTestApplication* wintc_patest_application_new(void)
{
    return WINTC_PATEST_APPLICATION(
        g_object_new(
            wintc_patest_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.play.patest",
            NULL
        )
    );
}
