#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSysAnalApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCSysAnalApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_sys_anal_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCSysAnalApplication,
    wintc_sys_anal_application,
    GTK_TYPE_APPLICATION
)

static void wintc_sys_anal_application_class_init(
    WinTCSysAnalApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_sys_anal_application_activate;
}

static void wintc_sys_anal_application_init(
    WINTC_UNUSED(WinTCSysAnalApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_sys_anal_application_activate(
    GApplication* application
)
{
    GtkWidget* new_window =
        wintc_sys_anal_window_new(WINTC_SYS_ANAL_APPLICATION(application));

    gtk_widget_show_all(new_window);
}

//
// PUBLIC FUNCTIONS
//
WinTCSysAnalApplication* wintc_sys_anal_application_new(void)
{
    return WINTC_SYS_ANAL_APPLICATION(
        g_object_new(
            wintc_sys_anal_application_get_type(),
            "application-id", "uk.co.oddmatics.wintc.play.sysanal",
            NULL
        )
    );
}
