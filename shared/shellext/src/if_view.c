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

gint wintc_ishext_view_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->compare_items(view, item_hash1, item_hash2);
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

GList* wintc_ishext_view_get_items(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_items(view);
}

GMenuModel* wintc_ishext_view_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_operations_for_item(view, item_hash);
}

GMenuModel* wintc_ishext_view_get_operations_for_view(
    WinTCIShextView* view
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->get_operations_for_view(view);
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

WinTCShextOperation* wintc_ishext_view_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
)
{
    WinTCIShextViewInterface* iface =
        WINTC_ISHEXT_VIEW_GET_IFACE(view);

    return iface->spawn_operation(
        view,
        operation_id,
        targets,
        error
    );
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

// HACKY SORT FUNC
static WinTCIShextView* S_CUR_VIEW_FOR_SORTING = NULL;

gint _wintc_ishext_view_sort_func(
    gconstpointer item_hash1,
    gconstpointer item_hash2
)
{
    if (!S_CUR_VIEW_FOR_SORTING)
    {
        g_critical("%s", "shellext: attempting to sort with no view!");
        return 0;
    }

    return wintc_ishext_view_compare_items(
        S_CUR_VIEW_FOR_SORTING,
        GPOINTER_TO_UINT(item_hash1),
        GPOINTER_TO_UINT(item_hash2)
    );
}

GCompareFunc wintc_ishext_view_get_sort_func(
    WinTCIShextView* view
)
{
    //
    // FIXME: This is NOT thread-safe!!
    //
    S_CUR_VIEW_FOR_SORTING = view;

    return ((GCompareFunc) _wintc_ishext_view_sort_func);
}
// END HACKY SORT FUNC

void wintc_shext_path_info_demangle_uri(
    WinTCShextPathInfo* path_info,
    const gchar*        uri
)
{
    //
    // Fun stuff! There's a bunch of 'special' shell stuff we cram into URIs
    // that aren't strictly valid... but they make sense to us -- this function
    // 'demangles' the special sauce back into a valid WinTCShextPathInfo
    //
    // This is useful for handling GApplication::open when you might expect to
    // handle a path incoming from WinTC
    //

    //
    // STEP 1: Check if this is a GUID path
    //
    const gchar* p_guid_str = strstr(uri, "::%7B");

    if (p_guid_str)
    {
        const gchar* p_end_guid = strstr(p_guid_str, "%7D");

        if (p_end_guid)
        {
            gchar* guid_escaped = wintc_substr(p_guid_str, p_end_guid + 3);

            path_info->base_path =
                g_uri_unescape_string(guid_escaped, NULL);

            g_free(guid_escaped);
        }
    }

    //
    // STEP 2: Check if there's an extended path
    //
    const gchar* p_ext_delim = strstr(uri, "??");

    if (p_ext_delim)
    {
        if (!path_info->base_path)
        {
            path_info->base_path =
                wintc_substr(uri, p_ext_delim);
        }

        path_info->extended_path =
            wintc_substr(p_ext_delim + 2, NULL);
    }

    //
    // STEP 3: If we still have no path up to this point, just put the whole
    //         thing in base_path and pass it through as-is
    //
    if (!path_info->base_path)
    {
        path_info->base_path = g_strdup(uri);
    }
}

gchar* wintc_shext_path_info_get_as_single_path(
    WinTCShextPathInfo* path_info
)
{
    if (path_info->extended_path)
    {
        return
            g_strdup_printf(
                "%s??%s",
                path_info->base_path,
                path_info->extended_path
            );
    }

    return g_strdup(path_info->base_path);
}

void wintc_shext_path_info_copy(
    WinTCShextPathInfo*       dst,
    const WinTCShextPathInfo* src
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

void wintc_shext_path_info_free(
    WinTCShextPathInfo* path_info
)
{
    wintc_shext_path_info_free_data(path_info);
    g_free(path_info);
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
