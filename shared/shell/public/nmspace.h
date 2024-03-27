#ifndef __SHELL_NMSPACE_H__
#define __SHELL_NMSPACE_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

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
