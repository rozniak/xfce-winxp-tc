#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/vwdrives.h"

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
static GHashTable* s_drives_map = NULL;

// FIXME: Temporary - only item is the drive root atm
//
static WinTCShextViewItem s_temp_items[] = {
    {
        "/",
        "drive-harddisk",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        "file:///"
    },
    {
        "Control Panel",
        "preferences-other",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL
    }
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_drives_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_drives_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static gboolean wintc_sh_view_drives_activate_item(
    WinTCIShextView*    view,
    guint               item_data,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_sh_view_drives_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static const gchar* wintc_sh_view_drives_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_drives_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_sh_view_drives_get_items(
    WinTCIShextView* view
);
static GMenuModel* wintc_sh_view_drives_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_sh_view_drives_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_sh_view_drives_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_sh_view_drives_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static guint wintc_sh_view_drives_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_sh_view_drives_has_parent(
    WinTCIShextView* view
);
static void wintc_sh_view_drives_refresh_items(
    WinTCIShextView* view
);
static WinTCShextOperation* wintc_sh_view_drives_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCShViewDrivesClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewDrives
{
    GObject __parent__;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewDrives,
    wintc_sh_view_drives,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_drives_ishext_view_interface_init
    )
)

static void wintc_sh_view_drives_class_init(
    WinTCShViewDrivesClass* klass
)
{
    s_drives_map = g_hash_table_new(g_direct_hash, g_direct_equal);

    // TEMP: Assign CPL view path
    //
    s_temp_items[1].priv = wintc_sh_path_for_guid(WINTC_SH_GUID_CPL);

    // TEMP: Assign hashes
    //
    s_temp_items[0].hash = g_str_hash("/");
    s_temp_items[1].hash = g_str_hash(s_temp_items[1].priv);

    // TEMP: Prepend items
    //
    g_hash_table_insert(
        s_drives_map,
        GUINT_TO_POINTER(s_temp_items[0].hash),
        &(s_temp_items[0])
    );
    g_hash_table_insert(
        s_drives_map,
        GUINT_TO_POINTER(s_temp_items[1].hash),
        &(s_temp_items[1])
    );

    // GObject initialisation
    //
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_sh_view_drives_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
}

static void wintc_sh_view_drives_init(
    WINTC_UNUSED(WinTCShViewDrives* self)
) {}

static void wintc_sh_view_drives_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_sh_view_drives_activate_item;
    iface->compare_items           = wintc_sh_view_drives_compare_items;
    iface->get_display_name        = wintc_sh_view_drives_get_display_name;
    iface->get_icon_name           = wintc_sh_view_drives_get_icon_name;
    iface->get_items               = wintc_sh_view_drives_get_items;
    iface->get_operations_for_item =
        wintc_sh_view_drives_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_sh_view_drives_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_drives_get_parent_path;
    iface->get_path                = wintc_sh_view_drives_get_path;
    iface->get_unique_hash         = wintc_sh_view_drives_get_unique_hash;
    iface->has_parent              = wintc_sh_view_drives_has_parent;
    iface->refresh_items           = wintc_sh_view_drives_refresh_items;
    iface->spawn_operation         = wintc_sh_view_drives_spawn_operation;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_drives_get_property(
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
static gboolean wintc_sh_view_drives_activate_item(
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
                s_drives_map,
                GUINT_TO_POINTER(item_hash)
            );

    // TODO: Handle properly, we're using temp items for now
    //
    path_info->base_path = g_strdup(item->priv);

    return TRUE;
}

static gint wintc_sh_view_drives_compare_items(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash1),
    WINTC_UNUSED(guint            item_hash2)
)
{
    // FIXME: Proper implementation
    return -1;
}

static const gchar* wintc_sh_view_drives_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Use shlang!
    //
    return "My Computer";
}

static const gchar* wintc_sh_view_drives_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "computer";
}

static GList* wintc_sh_view_drives_get_items(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_hash_table_get_values(s_drives_map);
}

static GMenuModel* wintc_sh_view_drives_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static GMenuModel* wintc_sh_view_drives_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static void wintc_sh_view_drives_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
        );
}

static void wintc_sh_view_drives_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DRIVES)
        );
}

static guint wintc_sh_view_drives_get_unique_hash(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_str_hash(wintc_sh_get_place_path(WINTC_SH_PLACE_DRIVES));
}

static gboolean wintc_sh_view_drives_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_sh_view_drives_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "shell: refresh drives view");

    _wintc_ishext_view_refreshing(view);

    // Emit only the root '/' for now
    // TODO: Basically everything in My Computer!
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(s_drives_map);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

static WinTCShextOperation* wintc_sh_view_drives_spawn_operation(
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
WinTCIShextView* wintc_sh_view_drives_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DRIVES,
            NULL
        )
    );
}
