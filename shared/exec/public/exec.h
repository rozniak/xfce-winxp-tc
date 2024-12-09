/** @file */

#ifndef __EXEC_EXEC_H__
#define __EXEC_EXEC_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Launches the specified command line.
 *
 * @param cmdline   The command line.
 * @param out_error Storage location for any error that occurred.
 * @return True if the command was launched successfully.
 */
gboolean wintc_launch_command(
    const gchar* cmdline,
    GError**     out_error
);

/**
 * Launches the specified command line synchronously.
 *
 * @param cmdline The command line.
 * @param standard_output Storage location for stdout.
 * @param standard_error  Storage location for stderr.
 * @param error           Storage location for any error that occurred.
 * @return True if the command was launched successfully.
 */
gboolean wintc_launch_command_sync(
    const gchar* cmdline,
    gchar**      standard_output,
    gchar**      standard_error,
    GError**     error
);

#endif
