#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/wizard97.h>

#include "netwiz.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupNetworkWizardClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCSetupNetworkWizard
{
    WinTCWizard97Window __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupNetworkWizard,
    wintc_setup_network_wizard,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_setup_network_wizard_class_init(
    WinTCSetupNetworkWizardClass* klass
)
{
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        "/uk/oddmatics/wintc/wsetupx/watermk.png",
        "/uk/oddmatics/wintc/wsetupx/header.png",
        "/uk/oddmatics/wintc/wsetupx/netwizp1.ui",
        "/uk/oddmatics/wintc/wsetupx/netwizp2.ui",
        "/uk/oddmatics/wintc/wsetupx/netwizp3.ui",
        NULL
    );
}

static void wintc_setup_network_wizard_init(
    WinTCSetupNetworkWizard* self
)
{
    wintc_wizard97_window_init_wizard(
         WINTC_WIZARD97_WINDOW(self)
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_setup_network_wizard_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SETUP_NETWORK_WIZARD,
            "title", "Windows XP Professional Setup",
            NULL
        )
    );
}
