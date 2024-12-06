#include <gio/gio.h>
#include <glib.h>

#include "../public/menu.h"

//
// FORWARD DECLARATIONS
//
static void menu_merge_model(
    GMenu*      menu,
    GMenuModel* menu_model
);

//
// PUBLIC FUNCTIONS
//
GMenuModel* wintc_menu_model_merge(
    GMenuModel* menu1,
    ...
)
{
    GMenu* menu = g_menu_new();

    menu_merge_model(menu, menu1);

    // Iterate over models
    //
    va_list     ap;
    GMenuModel* next_model;

    va_start(ap, menu1);

    next_model = va_arg(ap, GMenuModel*);

    while (next_model)
    {
        menu_merge_model(menu, next_model);

        next_model = va_arg(ap, GMenuModel*);
    }

    va_end(ap);

    return G_MENU_MODEL(menu);
}

//
// PRIVATE FUNCTIONS
//
static void menu_merge_model(
    GMenu*      menu,
    GMenuModel* menu_model
)
{
    // This seems really dumb... but in order to merge menus you essentially
    // have to copy TWICE - first to copy out into a new item, then copy that
    // item into the target menu
    //
    // I would've thought there be a better way, but I am not sure there is
    //
    gint n_items = g_menu_model_get_n_items(menu_model);

    for (gint i = 0; i < n_items; i++)
    {
        GMenuItem* menu_item = g_menu_item_new_from_model(menu_model, i);

        g_menu_append_item(menu, menu_item);

        g_object_unref(menu_item);
    }
}
