#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/vwdesk.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_ICON_NAME = 1
};

//
// STATIC DATA
//

// FIXME: Temp
//
static GHashTable* s_desktop_map = NULL;

// FIXME: LAZY AGAIN! Use shlang!!!!!! Temporary as well cos the user can
//        toggle which items are present
//
static WinTCShextViewItem s_desktop_items[] = {
    {
        "My Computer",
        "computer",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL
    },
    {
        "My Documents",
        "folder-documents",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL
    },
    {
        "My Network Places",
        "network-workgroup",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL,
    },
    {
        "Recycle Bin",
        "user-trash",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL,
    }
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_desktop_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_desktop_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static gboolean wintc_sh_view_desktop_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_sh_view_desktop_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static const gchar* wintc_sh_view_desktop_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_desktop_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_sh_view_desktop_get_items(
    WinTCIShextView* view
);
static GMenuModel* wintc_sh_view_desktop_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_sh_view_desktop_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_sh_view_desktop_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_sh_view_desktop_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static guint wintc_sh_view_desktop_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_sh_view_desktop_has_parent(
    WinTCIShextView* view
);
static void wintc_sh_view_desktop_refresh_items(
    WinTCIShextView* view
);
static WinTCShextOperation* wintc_sh_view_desktop_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);

//
// GLIB OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShViewDesktopClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewDesktop
{
    GObject __parent__;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewDesktop,
    wintc_sh_view_desktop,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_desktop_ishext_view_interface_init
    )
)

static void wintc_sh_view_desktop_class_init(
    WinTCShViewDesktopClass* klass
)
{
    s_desktop_map = g_hash_table_new(g_direct_hash, g_direct_equal);

    // Assign GUID paths to built-in desktop items - kind of rubbish but
    // whatever
    //
    s_desktop_items[0].priv = wintc_sh_path_for_guid(WINTC_SH_GUID_DRIVES);

    // Assign hashes
    //
    for (gulong i = 0; i < G_N_ELEMENTS(s_desktop_items); i++)
    {
        // FIXME: Temporary hack until the implementations are finished
        //
        if (s_desktop_items[i].priv)
        {
            s_desktop_items[i].hash = g_str_hash(s_desktop_items[i].priv);
        }
        else
        {
            gchar* temp = g_strdup_printf("desktop%d", g_random_int());

            s_desktop_items[i].hash = g_str_hash(temp);

            g_free(temp);
        }

        g_hash_table_insert(
            s_desktop_map,
            GUINT_TO_POINTER(s_desktop_items[i].hash),
            &(s_desktop_items[i])
        );
    }

    // GObject initialization
    //
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_sh_view_desktop_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
}

static void wintc_sh_view_desktop_init(
    WINTC_UNUSED(WinTCShViewDesktop* self)
) {}

static void wintc_sh_view_desktop_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_sh_view_desktop_activate_item;
    iface->compare_items           = wintc_sh_view_desktop_compare_items;
    iface->get_display_name        = wintc_sh_view_desktop_get_display_name;
    iface->get_icon_name           = wintc_sh_view_desktop_get_icon_name;
    iface->get_items               = wintc_sh_view_desktop_get_items;
    iface->get_operations_for_item =
        wintc_sh_view_desktop_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_sh_view_desktop_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_desktop_get_parent_path;
    iface->get_path                = wintc_sh_view_desktop_get_path;
    iface->get_unique_hash         = wintc_sh_view_desktop_get_unique_hash;
    iface->has_parent              = wintc_sh_view_desktop_has_parent;
    iface->refresh_items           = wintc_sh_view_desktop_refresh_items;
    iface->spawn_operation         = wintc_sh_view_desktop_spawn_operation;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_desktop_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCIShextView* view = WINTC_ISHEXT_VIEW(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            g_value_set_string(
                value,
                wintc_ishext_view_get_icon_name(view)
            );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// INTERFACE METHODS (WinTCIShextView)
//
static gboolean wintc_sh_view_desktop_activate_item(
    WINTC_UNUSED(WinTCIShextView* view),
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    WinTCShextViewItem* item =
        (WinTCShextViewItem*)
            g_hash_table_lookup(
                s_desktop_map,
                GUINT_TO_POINTER(item_hash)
            );

    if (!(item->priv))
    {
        g_critical("%s", "shell: desk view can't activate item, no target");
        return FALSE;
    }

    path_info->base_path = g_strdup(item->priv);

    return TRUE;
}

static gint wintc_sh_view_desktop_compare_items(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash1),
    WINTC_UNUSED(guint            item_hash2)
)
{
    // FIXME: Proper implementation
    return -1;
}

static const gchar* wintc_sh_view_desktop_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: LAZY!! Use shlang!
    return "Desktop";
}

static const gchar* wintc_sh_view_desktop_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "user-desktop";
}

static GList* wintc_sh_view_desktop_get_items(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_hash_table_get_values(s_desktop_map);
}

static GMenuModel* wintc_sh_view_desktop_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static GMenuModel* wintc_sh_view_desktop_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static void wintc_sh_view_desktop_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(WinTCShextPathInfo* path_info)
) {}

static void wintc_sh_view_desktop_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
        );
}

static guint wintc_sh_view_desktop_get_unique_hash(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_str_hash(wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP));
}

static gboolean wintc_sh_view_desktop_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return FALSE;
}

static void wintc_sh_view_desktop_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "shell: refresh desktop view");

    _wintc_ishext_view_refreshing(view);

    // Just emit the default items for now
    // TODO: Should aggregate with user desktop files
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(s_desktop_map);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

static WinTCShextOperation* wintc_sh_view_desktop_spawn_operation(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(gint             operation_id),
    WINTC_UNUSED(GList*           targets),
    WINTC_UNUSED(GError**         error)
)
{
    g_critical("Not implemented %s", __func__);
    return NULL;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_desktop_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DESKTOP,
            NULL
        )
    );
}
