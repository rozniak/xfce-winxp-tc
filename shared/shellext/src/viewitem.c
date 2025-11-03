#include <glib.h>

#include "../public/viewitem.h"

//
// PUBLIC FUNCTIONS
//
gint wintc_shext_view_item_compare_by_name(
    const WinTCShextViewItem* item1,
    const WinTCShextViewItem* item2
)
{
    return g_strcmp0(item1->display_name, item2->display_name);
}

gint wintc_shext_view_item_compare_by_fs_order(
    const WinTCShextViewItem* item1,
    const WinTCShextViewItem* item2
)
{
    if (item1->is_leaf == item2->is_leaf)
    {
        return wintc_shext_view_item_compare_by_name(item1, item2);
    }

    return item1->is_leaf ? 1 : -1;
}
