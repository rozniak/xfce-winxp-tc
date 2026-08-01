#ifndef __SHELL_NMSPACE_H__
#define __SHELL_NMSPACE_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// PUBLIC CONSTANTS
//
extern const gchar* WINTC_SH_GUID_CATEGORY_DRIVES;
extern const gchar* WINTC_SH_GUID_CATEGORY_LOCAL_FILES;
extern const gchar* WINTC_SH_GUID_CATEGORY_REMOVABLES;
extern const gchar* WINTC_SH_GUID_CATEGORY_OTHER;

//
// PUBLIC FUNCTIONS
//
gboolean wintc_sh_init_builtin_extensions(
    WinTCShextHost* shext_host
);

void wintc_sh_init_namespace_tree(
    GtkTreeModel*   tree_model,
    WinTCShextHost* shext_host
);

#endif
