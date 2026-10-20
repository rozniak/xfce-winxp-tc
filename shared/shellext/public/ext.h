/** @file */

#ifndef __SHELLEXT_EXT_H__
#define __SHELLEXT_EXT_H__

#include <glib.h>

//
// PUBLIC CALLBACK PROTOTYPES
//
typedef gboolean (*WinTCShextExtFunc) (
    gpointer iface,
    GError** error
);

//
// PUBLIC FUNCTIONS
//
WinTCShextExtFunc wintc_shext_ext_get_api(
    const gchar* lib_path
);

#endif
