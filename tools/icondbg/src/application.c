#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCIconDbgApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_icon_dbg_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCIconDbgApplication,
    wintc_icon_dbg_application,
    GTK_TYPE_APPLICATION
)

static void wintc_icon_dbg_application_class_init(
    WinTCIconDbgApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_icon_dbg_application_activate;
}

static void wintc_icon_dbg_application_init(
    WINTC_UNUSED(WinTCIconDbgApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_icon_dbg_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_icon_dbg_window_new(WINTC_ICON_DBG_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

//
// PUBLIC FUNCTIONS
//
WinTCIconDbgApplication* wintc_icon_dbg_application_new(void)
{
    return WINTC_ICON_DBG_APPLICATION(
        g_object_new(
            wintc_icon_dbg_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.samples.icondbg",
            NULL
        )
    );
}
