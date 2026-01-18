#ifndef __SHELL_NEWMENU_H__
#define __SHELL_NEWMENU_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
GMenuModel* wintc_sh_new_menu_get_menu(void);

gboolean wintc_sh_new_menu_create_file(
    const gchar* path,
    gint         op_id,
    guint*       hash_new_file,
    GError**     error
);

#endif
