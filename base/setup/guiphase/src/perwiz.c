#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/wizard97.h>

#include "perwiz.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupPersonalizeWizardClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCSetupPersonalizeWizard
{
    WinTCWizard97Window __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupPersonalizeWizard,
    wintc_setup_personalize_wizard,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_setup_personalize_wizard_class_init(
    WinTCSetupPersonalizeWizardClass* klass
)
{
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        "/uk/oddmatics/wintc/wsetupx/watermk.png",
        "/uk/oddmatics/wintc/wsetupx/header.png",
        "/uk/oddmatics/wintc/wsetupx/perwizp1.ui",
        "/uk/oddmatics/wintc/wsetupx/perwizp2.ui",
        "/uk/oddmatics/wintc/wsetupx/perwizp3.ui",
        "/uk/oddmatics/wintc/wsetupx/perwizp4.ui",
        "/uk/oddmatics/wintc/wsetupx/perwizp5.ui",
        "/uk/oddmatics/wintc/wsetupx/netwizp1.ui",
        "/uk/oddmatics/wintc/wsetupx/netwizp2.ui",
        "/uk/oddmatics/wintc/wsetupx/netwizp3.ui",
        NULL
    );
}

static void wintc_setup_personalize_wizard_init(
    WinTCSetupPersonalizeWizard* self
)
{
    wintc_wizard97_window_init_wizard(
         WINTC_WIZARD97_WINDOW(self)
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_personalize_wizard_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SETUP_PERSONALIZE_WIZARD,
            "title", "Windows XP Professional Setup",
            NULL
        )
    );
}
