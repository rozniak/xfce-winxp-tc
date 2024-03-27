#ifndef __SHCOMMON_PATH_H__
#define __SHCOMMON_PATH_H__

#include <glib.h>

#include "places.h"

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_sh_get_place_path(
    WinTCShPlace place
);

gchar* wintc_sh_path_for_guid(
    const gchar* guid
);

#endif
