/** @file */

#ifndef __COMGTK_ACCELERATOR_H__
#define __COMGTK_ACCELERATOR_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC STRUCTURES
//

/**
 * Convenience structure for defining accelerators for use with
 * wintc_application_set_accelerators.
 */
typedef struct _WinTCAccelEntry
{
    const gchar* action_name;
    const gchar* accelerator[2];
} WinTCAccelEntry;

//
// PUBLIC FUNCTIONS
//

/**
 * Convenience function for setting up many accelerators at once with
 * gtk_application_set_accels_for_action().
 *
 * @param application    The application.
 * @param accelerators   The array of accelerator entries.
 * @param n_accelerators The number of accelerators in the array.
 */
void wintc_application_set_accelerators(
    GtkApplication*        application,
    const WinTCAccelEntry* accelerators,
    guint                  n_accelerators
);

#endif
