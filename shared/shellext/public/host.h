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
typedef GtkWidget** (*WinTCShextPropertyPagesCtor) (
    const gchar* url,
    const gchar* mime_type
);
typedef WinTCIShextView* (*WinTCShextViewCtor) (
    WinTCShextViewAssoc assoc,
    const gchar*        assoc_str,
    const gchar*        url
);

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShextHostClass WinTCShextHostClass;
typedef struct _WinTCShextHost WinTCShextHost;

#define WINTC_TYPE_SHEXT_HOST            (wintc_shext_host_get_type())
#define WINTC_SHEXT_HOST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SHEXT_HOST, WinTCShextHost))
#define WINTC_SHEXT_HOST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SHEXT_HOST, WinTCShextHostClass))
#define IS_WINTC_SHEXT_HOST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SHEXT_HOST))
#define IS_WINTC_SHEXT_HOST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SHEXT_HOST))
#define WINTC_SHEXT_HOST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SHEXT_HOST, WinTCShextHost))

GType wintc_shext_host_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCShextHost* wintc_shext_host_new(void);

gboolean wintc_shext_host_add_toplevel_item(
    WinTCShextHost*     host,
    const gchar*        guid_category,
    WinTCShextViewItem* view_item,
    WinTCIShextView*    view,
    GError**            error
);

const WinTCShextCategory** wintc_shext_host_get_toplevel_categories(
    WinTCShextHost* host
);
const WinTCShextViewItem** wintc_shext_host_get_toplevel_items(
    WinTCShextHost* host,
    const gchar*    guid_category
);

WinTCIShextView* wintc_shext_host_get_view_for_path(
    WinTCShextHost*           host,
    const WinTCShextPathInfo* path_info,
    GError**                  error
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
