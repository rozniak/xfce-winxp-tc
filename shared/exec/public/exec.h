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
 * @param command   The command line.
 * @param out_error Storage location for any error that occurred.
 * @return True if the command was launched successfully.
 */
gboolean wintc_launch_command(
    const gchar* command,
    GError**     out_error
);

#endif
