#include <glib.h>
#include <gtk/gtk.h>

#include "../public/accelerator.h"

//
// PUBLIC FUNCTIONS
//
void wintc_application_set_accelerators(
    GtkApplication*        application,
    const WinTCAccelEntry* accelerators,
    guint                  n_accelerators
)
{
    for (guint i = 0; i < n_accelerators; i++)
    {
        gtk_application_set_accels_for_action(
            application,
            accelerators[i].action_name,
            accelerators[i].accelerator
        );
    }
}
