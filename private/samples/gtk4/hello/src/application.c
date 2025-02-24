#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"
#include "comgtk.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCHelloApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCHelloApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_hello_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCHelloApplication,
    wintc_hello_application,
    GTK_TYPE_APPLICATION
)

static void wintc_hello_application_class_init(
    WinTCHelloApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_hello_application_activate;
}

static void wintc_hello_application_init(
    WINTC_UNUSED(WinTCHelloApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_hello_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_hello_window_new(WINTC_HELLO_APPLICATION(application));

    gtk_window_present(GTK_WINDOW(new_window));
}

//
// PUBLIC FUNCTIONS
//
WinTCHelloApplication* wintc_hello_application_new(void)
{
    return WINTC_HELLO_APPLICATION(
        g_object_new(
            wintc_hello_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.samples.hello4",
            NULL
        )
    );
}
