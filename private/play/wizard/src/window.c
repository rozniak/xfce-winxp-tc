#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/wizard97.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWizardWindowClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCWizardWindow
{
    WinTCWizard97Window __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCWizardWindow,
    wintc_wizard_window,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_wizard_window_class_init(
    WinTCWizardWindowClass* klass
)
{
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        "/uk/oddmatics/wintc/play/wizard/header.png",
        "/uk/oddmatics/wintc/play/wizard/watermk.png",
        "/uk/oddmatics/wintc/play/wizard/wizpgext.ui",
        "/uk/oddmatics/wintc/play/wizard/wizpgint.ui"
    );
}

static void wintc_wizard_window_init(
    WinTCWizardWindow* self
)
{
    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_wizard_window_new(
    WinTCWizardApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_WIZARD_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "My Sample Wizard",
            NULL
        )
    );
}
