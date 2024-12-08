#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shell.h>

#include "application.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWinverApplicationClass
{
    GtkApplicationClass __parent__;
};

struct _WinTCWinverApplication
{
    GtkApplication __parent__;
};

//
// FORWARD DECLARATIONS
//
static void wintc_winver_application_activate(
    GApplication* application
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWinverApplication,
    wintc_winver_application,
    GTK_TYPE_APPLICATION
)

static void wintc_winver_application_class_init(
    WinTCWinverApplicationClass* klass
)
{
    GApplicationClass* application_class = G_APPLICATION_CLASS(klass);

    application_class->activate = wintc_winver_application_activate;
}

static void wintc_winver_application_init(
    WINTC_UNUSED(WinTCWinverApplication* self)
) { }

//
// CLASS VIRTUAL METHODS
//
static void wintc_winver_application_activate(
    WINTC_UNUSED(GApplication* application)
)
{
    wintc_sh_about(NULL, "Windows", NULL, NULL);
}

//
// PUBLIC FUNCTIONS
//
WinTCWinverApplication* wintc_winver_application_new(void)
{
    return WINTC_WINVER_APPLICATION(
        g_object_new(
            wintc_winver_application_get_type(),
            "application-id", "uk.oddmatics.wintc.winver",
            NULL
        )
    );
}
