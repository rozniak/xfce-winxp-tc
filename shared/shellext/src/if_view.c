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
}

//
// INTERFACE METHODS
//
gboolean wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->activate_item(
        view,
        item,
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
    WinTCIShextView*              view,
    WinTCShextViewItemsAddedData* items
)
{
    g_signal_emit(
        view,
        wintc_ishext_view_signals[SIGNAL_ITEMS_ADDED],
        0,
        items
    );
}

void _wintc_ishext_view_items_removed(
    WinTCIShextView*     view,
    WinTCShextViewItem** items
)
{
    g_signal_emit(
        view,
        wintc_ishext_view_signals[SIGNAL_ITEMS_REMOVED],
        0,
        items
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
