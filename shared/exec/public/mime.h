/** @file */

#ifndef __EXEC_MIME_H__
#define __EXEC_MIME_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Queries for the MIME type of the specified file.
 *
 * @param filepath The absolute path to the file.
 * @param out_error Storage location for any error that occurred.
 * @return The MIME type of the file, if one was detected.
 */
gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
);

/**
 * Queries for the program that is set as the default for opening resources
 * of the specified MIME type.
 *
 * @param mime_query The MIME type.
 * @param out_error  Storage location for any error that occurred.
 * @return A GDesktopAppInfo instance that represents the program.
 */
GDesktopAppInfo* wintc_query_mime_handler(
    const gchar* mime_query,
    GError**     out_error
);

#endif
