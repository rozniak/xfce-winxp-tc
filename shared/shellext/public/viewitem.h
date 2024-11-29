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
    guint    hash;
    gpointer priv;
} WinTCShextViewItem;

typedef struct _WinTCShextViewItemsUpdate
{
    GList*   data;
    gboolean done;
} WinTCShextViewItemsUpdate;

#endif
