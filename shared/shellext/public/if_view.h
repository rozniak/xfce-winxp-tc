/** @file */

#ifndef __SHELLEXT_IF_VIEW_H__
#define __SHELLEXT_IF_VIEW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "viewitem.h"

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShextPathInfo
{
    gchar* base_path;
    gchar* extended_path;
} WinTCShextPathInfo;

//
// GTK INTERFACE DEFINITIONS
//
#define WINTC_TYPE_ISHEXT_VIEW (wintc_ishext_view_get_type())

G_DECLARE_INTERFACE(
    WinTCIShextView,
    wintc_ishext_view,
    WINTC,
    ISHEXT_VIEW,
    GObject
)

struct _WinTCIShextViewInterface
{
    GTypeInterface base_iface;

    gboolean (*activate_item) (
        WinTCIShextView*    view,
        WinTCShextViewItem* item,
        WinTCShextPathInfo* path_info,
        GError**            error
    );

    void (*get_actions_for_item) (
        WinTCIShextView*    view,
        WinTCShextViewItem* item
    );
    void (*get_actions_for_view) (
        WinTCIShextView* view
    );

    const gchar* (*get_display_name) (
        WinTCIShextView* view
    );
    void (*get_parent_path) (
        WinTCIShextView*    view,
        WinTCShextPathInfo* path_info
    );
    void (*get_path) (
        WinTCIShextView*    view,
        WinTCShextPathInfo* path_info
    );
    gboolean (*has_parent) (
        WinTCIShextView* view
    );

    void (*refresh_items) (
        WinTCIShextView* view
    );
};

//
// INTERFACE METHODS
//
gboolean wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

void wintc_ishext_view_refresh_items(
    WinTCIShextView* view
);

void wintc_ishext_view_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);
void wintc_ishext_view_get_actions_for_view(
    WinTCIShextView* view
);

const gchar* wintc_ishext_view_get_display_name(
    WinTCIShextView* view
);
void wintc_ishext_view_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
void wintc_ishext_view_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
gboolean wintc_ishext_view_has_parent(
    WinTCIShextView* view
);

//
// PUBLIC FUNCTIONS
//
void _wintc_ishext_view_items_added(
    WinTCIShextView*              view,
    WinTCShextViewItemsAddedData* items
);
void _wintc_ishext_view_items_removed(
    WinTCIShextView*     view,
    WinTCShextViewItem** items
);

void wintc_shext_path_info_copy(
    WinTCShextPathInfo* dst,
    WinTCShextPathInfo* src
);
void wintc_shext_path_info_free_data(
    WinTCShextPathInfo* path_info
);
void wintc_shext_path_info_move(
    WinTCShextPathInfo* dst,
    WinTCShextPathInfo* src
);

#endif
