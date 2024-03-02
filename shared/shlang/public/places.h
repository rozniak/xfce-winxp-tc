/** @file */

#ifndef __SHLANG_PLACES_H__
#define __SHLANG_PLACES_H__

#include <glib.h>

//
// PUBLIC ENUMS
//

// TODO: This is loosely based off of the CSIDL constants - in future this enum should
//       probably be replaced by one in an actual shell library (an ACTUAL CSIDL enum)
//       and we simply provide translations based on THAT list
//
//       Of course some won't have any translation... COMMON_APPDATA should probably be
//       /var, which wouldn't be translated
//

/**
 * Specifies known locations in the shell/filesystem.
 */
typedef enum {
    WINTC_PLACE_APPDATA,
    WINTC_PLACE_DESKTOP,
    WINTC_PLACE_DOWNLOADS,
    WINTC_PLACE_FAVORITES,
    WINTC_PLACE_DOCUMENTS,
    WINTC_PLACE_MUSIC,
    WINTC_PLACE_PICTURES,
    WINTC_PLACE_RECENTS,
    WINTC_PLACE_RECYCLEBIN,
    WINTC_PLACE_VIDEO,
    WINTC_PLACE_ADMINTOOLS,
    WINTC_PLACE_DRIVES,
    WINTC_PLACE_NETHOOD,
    WINTC_PLACE_CONTROLPANEL,
    WINTC_PLACE_CONNECTIONS,
    WINTC_PLACE_PRINTERS
} WinTCShellPlace;

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
    WinTCShellPlace place
);

#endif
