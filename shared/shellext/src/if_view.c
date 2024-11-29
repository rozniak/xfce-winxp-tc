#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_view.h"
#include "../public/viewitem.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_ITEMS_ADDED = 0,
    SIGNAL_ITEMS_REMOVED,
    SIGNAL_REFRESHING,
    N_SIGNALS
};

//
// STATIC DATA
//
static gint wintc_ishext_view_signals[N_SIGNALS] = { 0 };

//
// GTK INTERFACE DEFINITIONS & CTORS
//
G_DEFINE_INTERFACE(
    WinTCIShextView,
    wintc_ishext_view,
    G_TYPE_OBJECT
)

static void wintc_ishext_view_default_init(
    WinTCIShextViewInterface* iface
)
{
    GType iface_type = G_TYPE_FROM_INTERFACE(iface);

    g_object_interface_install_property(
        iface,
        g_param_spec_string(
            "icon-name",
            "IconName",
            "The XDG icon name for the view.",
            "inode-directory",
            G_PARAM_READABLE
        )
    );

    wintc_ishext_view_signals[SIGNAL_ITEMS_ADDED] =
        g_signal_new(
            "items-added",
            iface_type,
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE,
            1,
            G_TYPE_POINTER
        );
    wintc_ishext_view_signals[SIGNAL_ITEMS_REMOVED] =
        g_signal_new(
            "items-removed",
            iface_type,
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__POINTER,
            G_TYPE_NONE,
            1,
            G_TYPE_POINTER
        );
    wintc_ishext_view_signals[SIGNAL_REFRESHING] =
        g_signal_new(
            "refreshing",
            iface_type,
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

//
// INTERFACE METHODS
//
gboolean wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->activate_item(
        view,
        item_hash,
        path_info,
        error
    );
}

void wintc_ishext_view_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    iface->get_actions_for_item(view, item);
}

void wintc_ishext_view_get_actions_for_view(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    iface->get_actions_for_view(view);
}

const gchar* wintc_ishext_view_get_display_name(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_display_name(view);
}

const gchar* wintc_ishext_view_get_icon_name(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_icon_name(view);
}

void wintc_ishext_view_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_parent_path(view, path_info);
}

void wintc_ishext_view_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_path(view, path_info);
}

guint wintc_ishext_view_get_unique_hash(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_unique_hash(view);
}

gboolean wintc_ishext_view_has_parent(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->has_parent(view);
}

void wintc_ishext_view_refresh_items(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    iface->refresh_items(view);
}

//
// PUBLIC FUNCTIONS
//
void _wintc_ishext_view_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update
)
{
    g_signal_emit(
        view,
        wintc_ishext_view_signals[SIGNAL_ITEMS_ADDED],
        0,
        update
    );
}

void _wintc_ishext_view_items_removed(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update
)
{
    g_signal_emit(
        view,
        wintc_ishext_view_signals[SIGNAL_ITEMS_REMOVED],
        0,
        update
    );
}

void _wintc_ishext_view_refreshing(
    WinTCIShextView* view
)
{
    g_signal_emit(
        view,
        wintc_ishext_view_signals[SIGNAL_REFRESHING],
        0
    );
}

void wintc_shext_path_info_copy(
    WinTCShextPathInfo* dst,
    WinTCShextPathInfo* src
)
{
    wintc_shext_path_info_free_data(dst);

    if (!src->base_path)
    {
        return;
    }

    dst->base_path = g_strdup(src->base_path);

    if (src->extended_path)
    {
        dst->extended_path = g_strdup(src->extended_path);
    }
}

void wintc_shext_path_info_free_data(
    WinTCShextPathInfo* path_info
)
{
    g_free(g_steal_pointer(&(path_info->base_path)));
    g_free(g_steal_pointer(&(path_info->extended_path)));
}

void wintc_shext_path_info_move(
    WinTCShextPathInfo* dst,
    WinTCShextPathInfo* src
)
{
    wintc_shext_path_info_free_data(dst);

    if (!src->base_path)
    {
        return;
    }

    dst->base_path     = g_steal_pointer(&(src->base_path));
    dst->extended_path = g_steal_pointer(&(src->extended_path));
}

guint wintc_shext_path_info_hash(
    gconstpointer v
)
{
    const WinTCShextPathInfo* path_info = (WinTCShextPathInfo*) v;

    // Not sure if this reduces the effectiveness of the hash... think it
    // should be okay?
    //
    guint hash1 = g_str_hash(
                      path_info->base_path ?
                        path_info->base_path : ""
                  );
    guint hash2 = g_str_hash(
                      path_info->extended_path ?
                        path_info->extended_path : ""
                  );

    return hash1 * 33 + hash2;
}

gboolean wintc_shext_path_info_equal(
    gconstpointer v1,
    gconstpointer v2
)
{
    const WinTCShextPathInfo* path_info1 = (WinTCShextPathInfo*) v1;
    const WinTCShextPathInfo* path_info2 = (WinTCShextPathInfo*) v2;

    return
        g_strcmp0(path_info1->base_path,     path_info2->base_path)     == 0 &&
        g_strcmp0(path_info1->extended_path, path_info2->extended_path) == 0;
}
