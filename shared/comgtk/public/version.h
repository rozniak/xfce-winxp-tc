/** @file */

#ifndef __COMGTK_VERSION_H__
#define __COMGTK_VERSION_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Identifies whether the installed version of WinTC is a debug build.
 *
 * @return True if WinTC is a debug build.
 */
gboolean wintc_build_is_debug(void);

/**
 * Retrieves the build tag identifying the installed version of WinTC.
 *
 * @return The WinTC build tag (caller is responsible for freeing).
 */
gchar* wintc_build_get_tag(void);

#endif
