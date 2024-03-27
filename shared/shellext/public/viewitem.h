/** @file */

#ifndef __SHELLEXT_VIEWITEM_H__
#define __SHELLEXT_VIEWITEM_H__

#include <glib.h>

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShextViewItem
{
    gchar*   display_name;
    gchar*   icon_name;
    gboolean is_leaf;
    gpointer priv;
} WinTCShextViewItem;

typedef struct _WinTCShextViewItemsAddedData
{
    WinTCShextViewItem* items;
    gint                num_items;
    gboolean            done;
} WinTCShextViewItemsAddedData;

#endif
