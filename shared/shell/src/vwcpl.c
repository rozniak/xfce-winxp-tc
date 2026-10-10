#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/actions.h"
#include "../public/cpl.h"
#include "../public/vwcpl.h"

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
static void wintc_sh_view_cpl_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_cpl_finalize(
    GObject* object
);
static void wintc_sh_view_cpl_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static gboolean wintc_sh_view_cpl_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_sh_view_cpl_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static GList* wintc_sh_view_cpl_drag_execute(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_cpl_drag_test(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_cpl_drop_execute(
    WinTCIShextView*   view,
    GtkWindow*         wnd,
    guint              item_hash,
    const char* const* uris,
    gboolean           hint_copy,
    GError**           error
);
static gboolean wintc_sh_view_cpl_drop_test(
    WinTCIShextView*    view,
    guint               item_hash,
    const gchar* const* uris
);
static GActionGroup* wintc_sh_view_cpl_get_actions(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_cpl_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_cpl_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_sh_view_cpl_get_items(
    WinTCIShextView* view
);
static GMenuModel* wintc_sh_view_cpl_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_sh_view_cpl_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_sh_view_cpl_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_sh_view_cpl_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static GMenuModel* wintc_sh_view_cpl_get_suggested_actions(
    WinTCIShextView* view,
    guint            item_hash
);
static guint wintc_sh_view_cpl_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_sh_view_cpl_has_parent(
    WinTCIShextView* view
);
static void wintc_sh_view_cpl_refresh_items(
    WinTCIShextView* view
);
static WinTCShextOperation* wintc_sh_view_cpl_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);
static void wintc_sh_view_cpl_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
);

static WinTCShextViewItem* wintc_sh_view_cpl_get_view_item(
    WinTCShViewCpl* view_cpl,
    guint           item_hash
);

static void action_open(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

//
// STATIC DATA
//
static GActionEntry S_CPL_ACTIONS[] = {
    {
        .name           = "open",
        .activate       = action_open,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }

    // TODO: explore
    // TODO: create-shortcut
};

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCShViewCplClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewCpl
{
    GObject __parent__;

    // State
    //
    GList*      cpls;
    GHashTable* map_items;

    GHashTable* map_actions_to_targets;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewCpl,
    wintc_sh_view_cpl,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_cpl_ishext_view_interface_init
    )
)

WINTC_SH_DEFINE_CB_VW_ACTIONS_CLEANUP(
    WinTCShViewCpl,
    view_cpl,
    map_actions_to_targets
)

static void wintc_sh_view_cpl_class_init(
    WinTCShViewCplClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_sh_view_cpl_finalize;
    object_class->get_property = wintc_sh_view_cpl_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
}

static void wintc_sh_view_cpl_init(
    WinTCShViewCpl* self
)
{
    WINTC_SH_INIT_VW_ACTIONS(self, map_actions_to_targets);
}

static void wintc_sh_view_cpl_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_sh_view_cpl_activate_item;
    iface->compare_items           = wintc_sh_view_cpl_compare_items;
    iface->drag_execute            = wintc_sh_view_cpl_drag_execute;
    iface->drag_test               = wintc_sh_view_cpl_drag_test;
    iface->drop_execute            = wintc_sh_view_cpl_drop_execute;
    iface->drop_test               = wintc_sh_view_cpl_drop_test;
    iface->get_actions             = wintc_sh_view_cpl_get_actions;
    iface->get_display_name        = wintc_sh_view_cpl_get_display_name;
    iface->get_icon_name           = wintc_sh_view_cpl_get_icon_name;
    iface->get_items               = wintc_sh_view_cpl_get_items;
    iface->get_operations_for_item = wintc_sh_view_cpl_get_operations_for_item;
    iface->get_operations_for_view = wintc_sh_view_cpl_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_cpl_get_parent_path;
    iface->get_path                = wintc_sh_view_cpl_get_path;
    iface->get_suggested_actions   = wintc_sh_view_cpl_get_suggested_actions;
    iface->get_unique_hash         = wintc_sh_view_cpl_get_unique_hash;
    iface->has_parent              = wintc_sh_view_cpl_has_parent;
    iface->refresh_items           = wintc_sh_view_cpl_refresh_items;
    iface->spawn_operation         = wintc_sh_view_cpl_spawn_operation;
    iface->update_actions          = wintc_sh_view_cpl_update_actions;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_cpl_finalize(
    GObject* object
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(object);

    g_clear_list(
        &(view_cpl->cpls),
        (GDestroyNotify) wintc_sh_cpl_applet_free
    );

    g_hash_table_destroy(
        g_steal_pointer(&(view_cpl->map_actions_to_targets))
    );

    if (view_cpl->map_items)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_cpl->map_items))
        );
    }

    (G_OBJECT_CLASS(wintc_sh_view_cpl_parent_class))->finalize(object);
}

static void wintc_sh_view_cpl_get_property(
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
static gboolean wintc_sh_view_cpl_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    WinTCShCplApplet*   applet;
    WinTCShextViewItem* item;

    item   = wintc_sh_view_cpl_get_view_item(view_cpl, item_hash);
    applet = (WinTCShCplApplet*) item->priv;

    if (wintc_sh_cpl_applet_is_executable(applet))
    {
        return wintc_launch_command(applet->exec, error);
    }

    path_info->base_path = g_strdup(applet->exec);

    return TRUE;
}

static gint wintc_sh_view_cpl_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    WinTCShextViewItem* item1 =
        wintc_sh_view_cpl_get_view_item(view_cpl, item_hash1);
    WinTCShextViewItem* item2 =
        wintc_sh_view_cpl_get_view_item(view_cpl, item_hash2);

    return wintc_shext_view_item_compare_by_name(item1, item2);
}

static GList* wintc_sh_view_cpl_drag_execute(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GList*              item_hashes),
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    //
    // FIXME: Create tmp files for shortcuts?
    //
    return NULL;
}

static gboolean wintc_sh_view_cpl_drag_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GList*              item_hashes),
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    //
    // FIXME: Handle creating shortcuts to the CPLs
    //
    return FALSE;
}

static gboolean wintc_sh_view_cpl_drop_execute(
    WINTC_UNUSED(WinTCIShextView*   view),
    WINTC_UNUSED(GtkWindow*         wnd),
    WINTC_UNUSED(guint              item_hash),
    WINTC_UNUSED(const char* const* uris),
    WINTC_UNUSED(gboolean           hint_copy),
    WINTC_UNUSED(GError**           error)
)
{
    return FALSE;
}

static gboolean wintc_sh_view_cpl_drop_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(const gchar* const* uris)
)
{
    // Control Panel never accepts drops
    //
    return FALSE;
}

static const gchar* wintc_sh_view_cpl_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Localisation
    //
    return "Control Panel";
}

static GActionGroup* wintc_sh_view_cpl_get_actions(
    WinTCIShextView* view
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    GSimpleActionGroup* actions = g_simple_action_group_new();

    WINTC_SH_CREATE_VW_ACTIONS_GROUP(
        view_cpl,
        map_actions_to_targets,
        actions,
        S_CPL_ACTIONS
    );

    return G_ACTION_GROUP(actions);
}

static const gchar* wintc_sh_view_cpl_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "preferences-other";
}

static GList* wintc_sh_view_cpl_get_items(
    WinTCIShextView* view
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    return g_hash_table_get_values(view_cpl->map_items);
}

static GMenuModel* wintc_sh_view_cpl_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menucplf.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    menu =
        G_MENU_MODEL(
            g_object_ref(
                gtk_builder_get_object(builder, "menu")
            )
        );

    g_object_unref(builder);

    return menu;
}

static GMenuModel* wintc_sh_view_cpl_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menucp.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    menu =
        G_MENU_MODEL(
            g_object_ref(
                gtk_builder_get_object(builder, "menu")
            )
        );

    g_object_unref(builder);

    return menu;
}

static void wintc_sh_view_cpl_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DRIVES)
        );
}

static void wintc_sh_view_cpl_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_CONTROLPANEL)
        );
}

static GMenuModel* wintc_sh_view_cpl_get_suggested_actions(
    WINTC_UNUSED(WinTCIShextView* view),
    guint item_hash
)
{
    if (item_hash)
    {
        g_critical(
            "%s",
            "shell: vwcpl suggested actions for item not implemented"
        );

        return NULL;
    }

    // Construct the suggested actions menu UI
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource("/uk/oddmatics/wintc/shell/amcplvw.ui");

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

static guint wintc_sh_view_cpl_get_unique_hash(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_str_hash(wintc_sh_get_place_path(WINTC_SH_PLACE_CONTROLPANEL));
}

static gboolean wintc_sh_view_cpl_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_sh_view_cpl_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    _wintc_ishext_view_refreshing(view);

    g_clear_list(
        &(view_cpl->cpls),
        (GDestroyNotify) wintc_sh_cpl_applet_free
    );

    if (view_cpl->map_items)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_cpl->map_items))
        );
    }

    // Create view items
    //
    view_cpl->cpls      = wintc_sh_cpl_applet_get_all();
    view_cpl->map_items = g_hash_table_new_full(
                              g_direct_hash,
                              g_direct_equal,
                              NULL,
                              (GDestroyNotify) g_free
                          );

    for (GList* iter = view_cpl->cpls; iter; iter = iter->next)
    {
        WinTCShCplApplet*   applet    = (WinTCShCplApplet*) iter->data;
        WinTCShextViewItem* view_item = g_new(WinTCShextViewItem, 1);

        view_item->display_name = applet->display_name;
        view_item->icon_name    = applet->icon_name ?
                                      applet->icon_name :
                                      "image-missing";
        view_item->is_leaf      = wintc_sh_cpl_applet_is_executable(applet);
        view_item->hash         = g_str_hash(applet->exec);
        view_item->priv         = applet;

        g_hash_table_insert(
            view_cpl->map_items,
            GUINT_TO_POINTER(view_item->hash),
            view_item
        );
    }

    // Provide update
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(view_cpl->map_items);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

static WinTCShextOperation* wintc_sh_view_cpl_spawn_operation(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(gint             operation_id),
    WINTC_UNUSED(GList*           targets),
    WINTC_UNUSED(GError**         error)
)
{
    g_critical("Not implemented %s", __func__);
    return NULL;
}

static void wintc_sh_view_cpl_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    WINTC_SH_UPDATE_VW_ACTIONS_GROUP(
        view_cpl,
        map_actions_to_targets,
        actions,
        items
    );

    //
    // FIXME: explore would be enabled/disabled here for shell folders
    //
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_cpl_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_CPL,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static WinTCShextViewItem* wintc_sh_view_cpl_get_view_item(
    WinTCShViewCpl* view_cpl,
    guint           item_hash
)
{
    return
        (WinTCShextViewItem*)
        g_hash_table_lookup(
            view_cpl->map_items,
            GUINT_TO_POINTER(item_hash)
        );
}

//
// CALLBACKS
//
static void action_open(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx = (WinTCShViewActions*) user_data;

    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(ctx->view);

    // Retrieve the targets
    // 
    GList* targets =
        g_hash_table_lookup(
            view_cpl->map_actions_to_targets,
            ctx->action_group
        );

    if (!targets)
    {
        g_critical("%s", "vwcpl: somehow launched 'open' with no targets");
    }

    for (GList* iter = targets; iter; iter = iter->next)
    {
        GError*            error     = NULL;
        WinTCShextPathInfo path_info = { 0 };

        if (
            wintc_sh_view_cpl_activate_item(
                ctx->view,
                GPOINTER_TO_UINT(iter->data),
                &path_info,
                &error
            )
        )
        {
            // If this was a shell folder, open in new window
            //
            if (path_info.base_path)
            {
                gchar* uri = wintc_shext_path_info_get_as_single_path(
                                 &path_info
                             );
                gchar* cmd = g_strdup_printf("explorer %s", uri);

                if (!wintc_launch_command(cmd, &error))
                {
                    wintc_display_error_and_clear(&error, NULL);
                }

                g_free(uri);
                g_free(cmd);
                wintc_shext_path_info_free_data(&path_info);
            }
        }
        else
        {
            wintc_display_error_and_clear(&error, NULL);
        }
    }
}
