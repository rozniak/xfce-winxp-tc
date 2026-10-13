#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "vwprntrs.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_ICON_NAME = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_cpl_view_printers_dispose(
    GObject* object
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
static GList* wintc_cpl_view_printers_drag_execute(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_cpl_view_printers_drag_test(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_cpl_view_printers_drop_execute(
    WinTCIShextView*    view,
    GtkWindow*          wnd,
    guint               item_hash,
    const gchar* const* uris,
    gboolean            hint_copy,
    GError**            error
);
static gboolean wintc_cpl_view_printers_drop_test(
    WinTCIShextView*    view,
    guint               item_hash,
    const gchar* const* uris
);
static GActionGroup* wintc_cpl_view_printers_get_actions(
    WinTCIShextView* view,
    GtkWindow*       wnd
);
static const gchar* wintc_cpl_view_printers_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_cpl_view_printers_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_cpl_view_printers_get_items(
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
static GMenuModel* wintc_cpl_view_printers_get_suggested_actions(
    WinTCIShextView* view,
    guint            item_hash
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
static void wintc_cpl_view_printers_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
);

void action_no_op(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry S_PRINTERS_ACTIONS[] =
{
    {
        .name           = "no-op",
        .activate       = action_no_op,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

// FIXME: Temp
//
static GHashTable* S_MAP_PRINTERS = NULL;

// FIXME: Temporary, until we display printers
//
static WinTCShextViewItem S_TEMP_ITEMS[] = {
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
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCCplViewPrintersClass
{
    GObjectClass __parent__;
};

struct _WinTCCplViewPrinters
{
    GObject __parent__;

    // State
    //
    GHashTable* map_actions_to_targets;
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

WINTC_SH_DEFINE_CB_VW_ACTIONS_CLEANUP(
    WinTCCplViewPrinters,
    view_printers,
    map_actions_to_targets
)

static void wintc_cpl_view_printers_class_init(
    WinTCCplViewPrintersClass* klass
)
{
    gchar* temp = g_strdup_printf("printers%d", g_random_int());

    S_TEMP_ITEMS[0].hash = g_str_hash(temp);

    g_free(temp);

    S_MAP_PRINTERS =
        g_hash_table_new(g_direct_hash, g_direct_equal);

    g_hash_table_insert(
        S_MAP_PRINTERS,
        GUINT_TO_POINTER(S_TEMP_ITEMS[0].hash),
        &(S_TEMP_ITEMS[0])
    );

    // GObject initialisation
    //
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_cpl_view_printers_dispose;
    object_class->get_property = wintc_cpl_view_printers_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
}

static void wintc_cpl_view_printers_init(
    WinTCCplViewPrinters* self
)
{
    WINTC_SH_INIT_VW_ACTIONS(self, map_actions_to_targets);
}

static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_cpl_view_printers_activate_item;
    iface->compare_items           = wintc_cpl_view_printers_compare_items;
    iface->drag_execute            = wintc_cpl_view_printers_drag_execute;
    iface->drag_test               = wintc_cpl_view_printers_drag_test;
    iface->drop_execute            = wintc_cpl_view_printers_drop_execute;
    iface->drop_test               = wintc_cpl_view_printers_drop_test;
    iface->get_actions             = wintc_cpl_view_printers_get_actions;
    iface->get_display_name        = wintc_cpl_view_printers_get_display_name;
    iface->get_icon_name           = wintc_cpl_view_printers_get_icon_name;
    iface->get_items               = wintc_cpl_view_printers_get_items;
    iface->get_operations_for_item =
        wintc_cpl_view_printers_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_cpl_view_printers_get_operations_for_view;
    iface->get_parent_path         = wintc_cpl_view_printers_get_parent_path;
    iface->get_path                = wintc_cpl_view_printers_get_path;
    iface->get_suggested_actions   =
        wintc_cpl_view_printers_get_suggested_actions;
    iface->get_unique_hash         = wintc_cpl_view_printers_get_unique_hash;
    iface->has_parent              = wintc_cpl_view_printers_has_parent;
    iface->refresh_items           = wintc_cpl_view_printers_refresh_items;
    iface->update_actions          = wintc_cpl_view_printers_update_actions;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_view_printers_dispose(
    GObject* object
)
{
    WinTCCplViewPrinters* view_printers = WINTC_CPL_VIEW_PRINTERS(object);

    WINTC_SH_DESTROY_VW_ACTIONS_MAP(view_printers, map_actions_to_targets);

    (G_OBJECT_CLASS(wintc_cpl_view_printers_parent_class))
        ->dispose(object);
}

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

static GList* wintc_cpl_view_printers_drag_execute(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GList*              item_hashes),
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    //
    // FIXME: Implement this
    //
    return NULL;
}

static gboolean wintc_cpl_view_printers_drag_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GList*              item_hashes),
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    //
    // FIXME: Implement this
    //
    return FALSE;
}

static gboolean wintc_cpl_view_printers_drop_execute(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GtkWindow*          wnd),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(const gchar* const* uris),
    WINTC_UNUSED(gboolean            hint_copy),
    WINTC_UNUSED(GError**            error)
)
{
    return FALSE;
}

static gboolean wintc_cpl_view_printers_drop_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(const gchar* const* uris)
)
{
    return FALSE;
}

static GActionGroup* wintc_cpl_view_printers_get_actions(
    WinTCIShextView* view,
    GtkWindow*       wnd
)
{
    WinTCCplViewPrinters* view_printers = WINTC_CPL_VIEW_PRINTERS(view);

    GSimpleActionGroup* actions = g_simple_action_group_new();

    WINTC_SH_CREATE_VW_ACTIONS_GROUP(
        view_printers,
        map_actions_to_targets,
        actions,
        S_PRINTERS_ACTIONS,
        wnd
    );

    return G_ACTION_GROUP(actions);
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

static GList* wintc_cpl_view_printers_get_items(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_hash_table_get_values(S_MAP_PRINTERS);
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

static GMenuModel* wintc_cpl_view_printers_get_suggested_actions(
    WINTC_UNUSED(WinTCIShextView* view),
    guint item_hash
)
{
    if (item_hash)
    {
        g_critical(
            "%s",
            "shell: vwprinters suggested actions for item not implemented"
        );

        return NULL;
    }

    // Construct the suggested actions menu UI
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/printers/amprntvw.ui"
        );

    GMenuModel* menu = NULL;

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "menu", &menu,
        NULL
    );

    g_object_ref(menu);

    g_object_unref(builder);

    return menu;
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

    GList* items = g_hash_table_get_values(S_MAP_PRINTERS);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

static void wintc_cpl_view_printers_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
)
{
    WinTCCplViewPrinters* view_printers = WINTC_CPL_VIEW_PRINTERS(view);

    WINTC_SH_UPDATE_VW_ACTIONS_GROUP(
        view_printers,
        map_actions_to_targets,
        actions,
        items
    );
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

//
// CALLBACKS
//
void action_no_op(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    WINTC_UNUSED(gpointer user_data)
)
{
    g_critical("%s", "vwprntrs: no op action called");
}
