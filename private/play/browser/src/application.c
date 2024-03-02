#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCBrowserApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCBrowserApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_browser_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCBrowserApplication,
    wintc_browser_application,
    GTK_TYPE_APPLICATION
)

static void wintc_browser_application_class_init(
    WinTCBrowserApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_browser_application_activate;
}

static void wintc_browser_application_init(
    WINTC_UNUSED(WinTCBrowserApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_browser_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_browser_window_new(WINTC_BROWSER_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

//
// PUBLIC FUNCTIONS
//
WinTCBrowserApplication* wintc_browser_application_new(void)
{
    return WINTC_BROWSER_APPLICATION(
        g_object_new(
            wintc_browser_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.play.browser",
            NULL
        )
    );
}
