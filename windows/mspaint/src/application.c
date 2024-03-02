#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCPaintApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCPaintApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_paint_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCPaintApplication,
    wintc_paint_application,
    GTK_TYPE_APPLICATION
)

static void wintc_paint_application_class_init(
    WinTCPaintApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_paint_application_activate;
}

static void wintc_paint_application_init(
    WINTC_UNUSED(WinTCPaintApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_paint_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_paint_window_new(WINTC_PAINT_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

//
// PUBLIC FUNCTIONS
//
WinTCPaintApplication* wintc_paint_application_new(void)
{
    return WINTC_PAINT_APPLICATION(
        g_object_new(
            wintc_paint_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.mspaint",
            NULL
        )
    );
}
