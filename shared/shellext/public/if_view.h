/** @file */

#ifndef __SHELLEXT_IF_VIEW_H__
#define __SHELLEXT_IF_VIEW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "viewitem.h"
#include "viewops.h"

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
        guint               item_hash,
        WinTCShextPathInfo* path_info,
        GError**            error
    );

    gint (*compare_items) (
        WinTCIShextView* view,
        guint            item_hash1,
        guint            item_hash2
    );
    const gchar* (*get_display_name) (
        WinTCIShextView* view
    );
    const gchar* (*get_icon_name) (
        WinTCIShextView* view
    );
    GList* (*get_items) (
        WinTCIShextView* view
    );
    GMenuModel* (*get_operations_for_item) (
        WinTCIShextView* view,
        guint            item_hash
    );
    GMenuModel* (*get_operations_for_view) (
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
    guint (*get_unique_hash) (
        WinTCIShextView* view
    );
    gboolean (*has_parent) (
        WinTCIShextView* view
    );

    void (*refresh_items) (
        WinTCIShextView* view
    );

    WinTCShextOperation* (*spawn_operation) (
        WinTCIShextView* view,
        gint             operation_id,
        GList*           targets,
        GError**         error
    );
};

//
// INTERFACE METHODS
//
gboolean wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);

gint wintc_ishext_view_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
void wintc_ishext_view_refresh_items(
    WinTCIShextView* view
);

const gchar* wintc_ishext_view_get_display_name(
    WinTCIShextView* view
);
const gchar* wintc_ishext_view_get_icon_name(
    WinTCIShextView* view
);
GList* wintc_ishext_view_get_items(
    WinTCIShextView* view
);
GMenuModel* wintc_ishext_view_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
GMenuModel* wintc_ishext_view_get_operations_for_view(
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
guint wintc_ishext_view_get_unique_hash(
    WinTCIShextView* view
);
gboolean wintc_ishext_view_has_parent(
    WinTCIShextView* view
);
WinTCShextOperation* wintc_ishext_view_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);

//
// PUBLIC FUNCTIONS
//
void _wintc_ishext_view_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update
);
void _wintc_ishext_view_items_removed(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update
);
void _wintc_ishext_view_refreshing(
    WinTCIShextView* view
);

GCompareFunc wintc_ishext_view_get_sort_func(
    WinTCIShextView* view
);

// WinTCShextPathInfo methods
//
void wintc_shext_path_info_demangle_uri(
    WinTCShextPathInfo* path_info,
    const gchar*        uri
);
gchar* wintc_shext_path_info_get_as_single_path(
    WinTCShextPathInfo* path_info
);

void wintc_shext_path_info_copy(
    WinTCShextPathInfo*       dst,
    const WinTCShextPathInfo* src
);
void wintc_shext_path_info_free(
    WinTCShextPathInfo* path_info
);
void wintc_shext_path_info_free_data(
    WinTCShextPathInfo* path_info
);
void wintc_shext_path_info_move(
    WinTCShextPathInfo* dst,
    WinTCShextPathInfo* src
);

guint wintc_shext_path_info_hash(
    gconstpointer v
);
gboolean wintc_shext_path_info_equal(
    gconstpointer v1,
    gconstpointer v2
);

#endif
