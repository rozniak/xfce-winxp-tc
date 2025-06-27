/** @file */

#ifndef __COMGTK_VERSION_H__
#define __COMGTK_VERSION_H__

#include <glib.h>

//
// PUBLIC ENUMS
//
enum
{
    WINTC_VER_PRETTY_NAME,
    WINTC_VER_NAME,
    WINTC_VER_SKU,
    WINTC_VER_SKU_TAGLINE,

    WINTC_VER_MAJOR,
    WINTC_VER_MINOR,
    WINTC_VER_BUILD,
    WINTC_VER_DATETIME,

    WINTC_VER_BRANCH,
    WINTC_VER_DATESTAMP,
    WINTC_VER_HASH,
    WINTC_VER_USER,

    WINTC_VER_TAG,
    WINTC_VER_PROJECT
};

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
 * Queries information related to the installed version of WinTC.
 *
 * @param query_id The ID of the version property to query.
 * @return The string value corresponding to the query ID.
 */
const gchar* wintc_build_query(
    guint query_id
);

#endif
