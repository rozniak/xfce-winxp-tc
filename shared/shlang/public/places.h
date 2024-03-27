/** @file */

#ifndef __SHLANG_PLACES_H__
#define __SHLANG_PLACES_H__

#include <glib.h>
#include <wintc/shcommon.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieves the translated text for a place name in the current locale.
 *
 * @param place The place.
 * @return The translated text.
 */
const gchar* wintc_lc_get_place_name(
    WinTCShPlace place
);

#endif
