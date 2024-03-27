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
WinTCShextPathInfo* wintc_ishext_view_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    GError**            error
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->activate_item(
        view,
        item,
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

const gchar* wintc_ishext_view_get_parent_path(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_parent_path(view);
}

const gchar* wintc_ishext_view_get_path(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_path(view);
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

void wintc_shext_path_info_free(
    WinTCShextPathInfo* path_info
)
{
    g_free(path_info->base_path);
    g_free(path_info->extended_path);
    g_free(path_info);
}
