/** @file */

#ifndef __COMGTK_PROFILE_H__
#define __COMGTK_PROFILE_H__

#include <glib.h>

/**
 * @def WINTC_COMPONENT_SHELL
 * The identifier for the shell component.
 */
#define WINTC_COMPONENT_SHELL "shell"

//
// PUBLIC FUNCTIONS
//

/**
 * Ensures that the directory structure for a component's data exists in the
 * user's profile.
 *
 * @param component The component's identifier.
 * @param out_error Storage location for any error that occurred.
 * @return True if the directory structure exists.
 */
gboolean wintc_profile_ensure_exists(
    const gchar* component,
    GError**     out_error
);

/**
 * Constructs the absolute path for the location of a component's data in the
 * user's profile.
 *
 * @param component The component's identifier.
 * @param filename  The name of the file in the profile.
 * @return The absolute path for the file in the user's profile.
 */
gchar* wintc_profile_get_path(
    const gchar* component,
    const gchar* filename
);

/**
 * Retrieves the contents of a file in the user's profile.
 *
 * @param component The component's identifier.
 * @param filename  The name of the file in the profile.
 * @param contents  Storage location for the file contents.
 * @param length    Storage location for the size of the file.
 * @param error     Storage location for any error that occurred.
 * @return True if the file was successfully read.
 */
gboolean wintc_profile_get_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar**      contents,
    gsize*       length,
    GError**     error
);

/**
 * Writes contents to a file in the user's profile.
 *
 * @param component The component's identifier.
 * @param filename  The name of the file in the profile.
 * @param contents  The contents to write.
 * @param length    The length of the contents, -1 for NULL terminated.
 * @param error     Storage location for any error that occurred.
 * @return True if the file was successfully written.
 */
gboolean wintc_profile_set_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar*       contents,
    gssize       length,
    GError**     error
);

#endif
