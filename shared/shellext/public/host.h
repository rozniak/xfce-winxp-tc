/** @file */

#ifndef __SHELLEXT_HOST_H__
#define __SHELLEXT_HOST_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "category.h"
#include "if_view.h"
#include "viewitem.h"

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SHEXT_LOAD_DEFAULT,
    WINTC_SHEXT_LOAD_ONLY,
    WINTC_SHEXT_LOAD_EXCLUDING
} WinTCShextLoadMode;

typedef enum
{
    WINTC_SHEXT_VIEW_ASSOC_DEFAULT,
    WINTC_SHEXT_VIEW_ASSOC_VIEW_GUID,
    WINTC_SHEXT_VIEW_ASSOC_MIME,
    WINTC_SHEXT_VIEW_ASSOC_REGEX,
    WINTC_SHEXT_VIEW_ASSOC_PATH
} WinTCShextViewAssoc;

//
// PUBLIC CALLBACK PROTOTYPES
//
typedef struct _WinTCShextHost WinTCShextHost;

typedef GtkWidget** (*WinTCShextPropertyPagesCtor) (
    WinTCShextHost* shext_host,
    const gchar*    url,
    const gchar*    mime_type
);
typedef WinTCIShextView* (*WinTCShextViewCtor) (
    WinTCShextHost*           shext_host,
    WinTCShextViewAssoc       assoc,
    const gchar*              assoc_str,
    const WinTCShextPathInfo* path_info
);
typedef gboolean (*WinTCShextActivateItemFunc) (
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

//
// PUBLIC STRUCTURES
//
typedef struct _WinTCShextTopLevelItem
{
    WinTCShextViewItem*        item;
    WinTCShextActivateItemFunc activate_cb;
    WinTCShextCategory*        category;
} WinTCShextTopLevelItem;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SHEXT_HOST (wintc_shext_host_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShextHost,
    wintc_shext_host,
    WINTC,
    SHEXT_HOST,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShextHost* wintc_shext_host_new(void);

gboolean wintc_shext_host_add_toplevel_item(
    WinTCShextHost*            host,
    const gchar*               guid_category,
    const gchar*               id,
    WinTCShextViewItem*        view_item,
    WinTCShextActivateItemFunc activate_cb,
    GError**                   error
);

GList* wintc_shext_host_get_toplevel_categories(
    WinTCShextHost* host
);
GList* wintc_shext_host_get_toplevel_items(
    WinTCShextHost* host,
    const gchar*    guid_category
);

WinTCIShextView* wintc_shext_host_get_view_for_path(
    WinTCShextHost*           host,
    const WinTCShextPathInfo* path_info,
    GError**                  error
);

gboolean wintc_shext_host_has_view_for_mime(
    WinTCShextHost* host,
    const gchar*    mime_type
);

gboolean wintc_shext_host_load_extensions(
    WinTCShextHost*    host,
    WinTCShextLoadMode mode,
    const gchar**      guid_filter
);

gboolean wintc_shext_host_register_property_pages_cb(
    WinTCShextHost*             host,
    const gchar*                mime_type,
    WinTCShextPropertyPagesCtor pages_cb
);
gboolean wintc_shext_host_register_toplevel_category(
    WinTCShextHost* host,
    const gchar*    guid,
    const gchar*    display_name
);
gboolean wintc_shext_host_register_view(
    WinTCShextHost*    host,
    const gchar*       guid,
    WinTCShextViewCtor factory_cb
);

void wintc_shext_host_remove_toplevel_item(
    WinTCShextHost* host,
    const gchar*    guid_category,
    const gchar*    id
);

gboolean wintc_shext_host_use_view_for_mime(
    WinTCShextHost*    host,
    const gchar*       mime_type,
    WinTCShextViewCtor factory_cb
);
gboolean wintc_shext_host_use_view_for_path_regex(
    WinTCShextHost*    host,
    GRegex*            regex,
    WinTCShextViewCtor factory_cb
);
gboolean wintc_shext_host_use_view_for_real_path(
    WinTCShextHost*    host,
    const gchar*       path,
    WinTCShextViewCtor factory_cb
);

#endif
