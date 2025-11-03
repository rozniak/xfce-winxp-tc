#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>

#include "vwprntrs.h"

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
static GHashTable* s_printers_map = NULL;

// FIXME: Temporary, until we display printers
//
static WinTCShextViewItem s_temp_items[] = {
    {
        "FPO",
        "printer",
        TRUE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL
    }
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_cpl_view_printers_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static gboolean wintc_cpl_view_printers_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_cpl_view_printers_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static const gchar* wintc_cpl_view_printers_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_cpl_view_printers_get_icon_name(
    WinTCIShextView* view
);
static GMenuModel* wintc_cpl_view_printers_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_cpl_view_printers_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_cpl_view_printers_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_cpl_view_printers_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static guint wintc_cpl_view_printers_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_cpl_view_printers_has_parent(
    WinTCIShextView* view
);
static void wintc_cpl_view_printers_refresh_items(
    WinTCIShextView* view
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCCplViewPrintersClass
{
    GObjectClass __parent__;
};

struct _WinTCCplViewPrinters
{
    GObject __parent__;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCCplViewPrinters,
    wintc_cpl_view_printers,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_cpl_view_printers_ishext_view_interface_init
    )
)

static void wintc_cpl_view_printers_class_init(
    WinTCCplViewPrintersClass* klass
)
{
    gchar* temp = g_strdup_printf("printers%d", g_random_int());

    s_temp_items[0].hash = g_str_hash(temp);

    g_free(temp);

    s_printers_map =
        g_hash_table_new(g_direct_hash, g_direct_equal);

    g_hash_table_insert(
        s_printers_map,
        GUINT_TO_POINTER(s_temp_items[0].hash),
        &(s_temp_items[0])
    );

    // GObject initialisation
    //
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_cpl_view_printers_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
}

static void wintc_cpl_view_printers_init(
    WINTC_UNUSED(WinTCCplViewPrinters* self)
) {}

static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_cpl_view_printers_activate_item;
    iface->compare_items           = wintc_cpl_view_printers_compare_items;
    iface->get_display_name        = wintc_cpl_view_printers_get_display_name;
    iface->get_icon_name           = wintc_cpl_view_printers_get_icon_name;
    iface->get_operations_for_item =
        wintc_cpl_view_printers_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_cpl_view_printers_get_operations_for_view;
    iface->get_parent_path         = wintc_cpl_view_printers_get_parent_path;
    iface->get_path                = wintc_cpl_view_printers_get_path;
    iface->get_unique_hash         = wintc_cpl_view_printers_get_unique_hash;
    iface->has_parent              = wintc_cpl_view_printers_has_parent;
    iface->refresh_items           = wintc_cpl_view_printers_refresh_items;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_view_printers_get_property(
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
static gboolean wintc_cpl_view_printers_activate_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    GError** error
)
{
    WINTC_SAFE_REF_CLEAR(error);
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

static gint wintc_cpl_view_printers_compare_items(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash1),
    WINTC_UNUSED(guint            item_hash2)
)
{
    // FIXME: Proper implementation
    return -1;
}

static GMenuModel* wintc_cpl_view_printers_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static GMenuModel* wintc_cpl_view_printers_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static const gchar* wintc_cpl_view_printers_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Use shlang
    //
    return "Printers and Faxes";
}

static const gchar* wintc_cpl_view_printers_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "uk.co.oddmatics.wintc.cpl-printers";
}

static void wintc_cpl_view_printers_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_CONTROLPANEL)
        );
}

static void wintc_cpl_view_printers_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_PRINTERS)
        );
}

static guint wintc_cpl_view_printers_get_unique_hash(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_str_hash(wintc_sh_get_place_path(WINTC_SH_PLACE_PRINTERS));
}

static gboolean wintc_cpl_view_printers_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_cpl_view_printers_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "cpl-prntrs: refresh printers view");

    _wintc_ishext_view_refreshing(view);

    // Emit the FPO printer for now
    // TODO: Implement this!
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(s_printers_map);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_cpl_view_printers_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_CPL_VIEW_PRINTERS,
            NULL
        )
    );
}
