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
 * Retrieves the 'pretty name' of the installed version of WinTC.
 *
 * @return The 'pretty name' of the installed version of WinTC.
 */
const gchar* wintc_build_get_pretty_name(void);

/**
 * Retrieves the edition of the installed version of WinTC.
 *
 * @return The edition of the installed version of WinTC.
 */
const gchar* wintc_build_get_sku_edition(void);

/**
 * Retrieves the name of the installed version of WinTC.
 *
 * @return The name of the installed version of WinTC.
 */
const gchar* wintc_build_get_sku_name(void);

/**
 * Retrieves the build tag identifying the installed version of WinTC.
 *
 * @return The WinTC build tag.
 */
const gchar* wintc_build_get_tag(void);

/**
 * Retrieves the tagline of the installed version of WinTC.
 *
 * @return The tagline of the installed version of WinTC.
 */
const gchar* wintc_build_get_tagline(void);

#endif
