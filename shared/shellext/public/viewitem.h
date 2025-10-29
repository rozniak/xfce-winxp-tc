/** @file */

#ifndef __SHELLEXT_VIEWITEM_H__
#define __SHELLEXT_VIEWITEM_H__

#include <glib.h>

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SHEXT_VIEW_ITEM_DEFAULT = 0,
    WINTC_SHEXT_VIEW_ITEM_IS_NEW
} WinTCShextViewItemHint;

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShextViewItem
{
    gchar*                 display_name;
    gchar*                 icon_name;
    gboolean               is_leaf;
    guint                  hash;
    WinTCShextViewItemHint hint;
    gpointer               priv;
} WinTCShextViewItem;

typedef struct _WinTCShextViewItemsUpdate
{
    GList*   data;
    gboolean done;
} WinTCShextViewItemsUpdate;

#endif
