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

    WinTCShextPathInfo* (*activate_item) (
        WinTCIShextView*    view,
        WinTCShextViewItem* item,
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
    const gchar* (*get_parent_path) (
        WinTCIShextView* view
    );
    const gchar* (*get_path) (
        WinTCIShextView* view
    );

    void (*refresh_items) (
        WinTCIShextView* view
    );
};

//
// INTERFACE METHODS
//
WinTCShextPathInfo* wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
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
const gchar* wintc_ishext_view_get_parent_path(
    WinTCIShextView* view
);
const gchar* wintc_ishext_view_get_path(
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

void wintc_shext_path_info_free(
    WinTCShextPathInfo* path_info
);

#endif
