/** @file */

#ifndef __SHELLEXT_EXT_H__
#define __SHELLEXT_EXT_H__

#include <glib.h>

#include "if_shwnd.h"

//
// PUBLIC CALLBACK PROTOTYPES
//
typedef gboolean (*WinTCShextExtFunc) (
    gpointer iface,
    GError** error
);

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShextToolbarInfo
{
    WinTCShextExtFunc fn_shext;
    gchar*            name;
    gchar*            soname;
    GtkWidget*        toolbar;
} WinTCShextToolbarInfo;

//
// PUBLIC FUNCTIONS
//
WinTCShextExtFunc wintc_shext_ext_get_api(
    const gchar* lib_path
);

GList* wintc_shext_ext_get_toolbar_apis(
    WinTCShextShellWindowKind shwnd_kind
);

void wintc_shext_toolbar_info_free(
    WinTCShextToolbarInfo* toolbar_info
);

#endif
