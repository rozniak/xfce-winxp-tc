/** @file */

#ifndef __COMGTK_MENU_H__
#define __COMGTK_MENU_H__

#include <gio/gio.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Merges multiple menu models to create a new one.
 *
 * @param menu1 The first menu.
 * @param ...   The other menus to merge, followed by NULL.
 * @return A new menu, created by merging the provided ones.
 */
GMenuModel* wintc_menu_model_merge(
    GMenuModel* menu1,
    ...
);

#endif
