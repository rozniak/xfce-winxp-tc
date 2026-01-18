#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/newmenu.h"
#include "../public/vwdesk.h"

#define K_ORDER_FS_ITEM 1717

//
// PRIVATE ENUMS
//
enum
{
    PROP_ICON_NAME = 1,
    PROP_SHEXT_HOST
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_desktop_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_desktop_constructed(
    GObject* object
);
static void wintc_sh_view_desktop_dispose(
    GObject* object
);
static void wintc_sh_view_desktop_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_view_desktop_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
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

static gint wintc_sh_view_desktop_get_item_order(
    guint item_hash
);
static GList* wintc_sh_view_desktop_filter_op_targets(
    WinTCShViewDesktop* view_desk,
    GList*              targets
);
static WinTCShextViewItem* wintc_sh_view_desktop_get_view_item(
    WinTCShViewDesktop* view_desk,
    guint               item_hash
);
static void wintc_sh_view_desktop_real_refresh_items(
    WinTCShViewDesktop* view_desk
);

static gboolean shopr_properties(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
);

static void on_view_user_desktop_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);
static void on_view_user_desktop_items_removed(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);
static void on_view_user_desktop_refreshing(
    WinTCIShextView* view,
    gpointer         user_data
);

//
// STATIC DATA
//
static GHashTable* S_DESKTOP_MAP   = NULL;
static GHashTable* S_DESKTOP_ORDER = NULL;

// FIXME: LAZY AGAIN! Use shlang!!!!!! Temporary as well cos the user can
//        toggle which items are present
//
static WinTCShextViewItem S_DESKTOP_ITEMS[] = {
    {
        "My Documents",
        "folder-documents",
        FALSE,
        0,
        WINTC_SHEXT_VIEW_ITEM_DEFAULT,
        NULL
    },
    {
        "My Computer",
        "computer",
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
// GLIB OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShViewDesktopClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewDesktop
{
    GObject __parent__;

    // State
    //
    WinTCShextHost*  shext_host;
    WinTCIShextView* view_user_desktop;
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
    S_DESKTOP_MAP   = g_hash_table_new(g_direct_hash, g_direct_equal);
    S_DESKTOP_ORDER = g_hash_table_new(g_direct_hash, g_direct_equal);

    // Assign GUID paths to built-in desktop items - kind of rubbish but
    // whatever
    //
    S_DESKTOP_ITEMS[1].priv = wintc_sh_path_for_guid(WINTC_SH_GUID_DRIVES);

    // Assign hashes
    //
    for (gulong i = 0; i < G_N_ELEMENTS(S_DESKTOP_ITEMS); i++)
    {
        // FIXME: Temporary hack until the implementations are finished
        //
        if (S_DESKTOP_ITEMS[i].priv)
        {
            S_DESKTOP_ITEMS[i].hash = g_str_hash(S_DESKTOP_ITEMS[i].priv);
        }
        else
        {
            gchar* temp = g_strdup_printf("desktop%d", g_random_int());

            S_DESKTOP_ITEMS[i].hash = g_str_hash(temp);

            g_free(temp);
        }

        g_hash_table_insert(
            S_DESKTOP_MAP,
            GUINT_TO_POINTER(S_DESKTOP_ITEMS[i].hash),
            &(S_DESKTOP_ITEMS[i])
        );

        // Map the item order (for compare_items)
        //
        g_hash_table_insert(
            S_DESKTOP_ORDER,
            GUINT_TO_POINTER(S_DESKTOP_ITEMS[i].hash),
            GINT_TO_POINTER(i + 1) // Add 1 to simplify later, so NULL/0 means
                                   // it's an FS item
        );
    }

    // GObject initialization
    //
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_view_desktop_constructed;
    object_class->dispose      = wintc_sh_view_desktop_dispose;
    object_class->get_property = wintc_sh_view_desktop_get_property;
    object_class->set_property = wintc_sh_view_desktop_set_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );
    
    g_object_class_install_property(
        object_class,
        PROP_SHEXT_HOST,
        g_param_spec_object(
            "shext-host",
            "ShextHost",
            "The shell extension host instance.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
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
static void wintc_sh_view_desktop_constructed(
    GObject* object
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(object);

    (G_OBJECT_CLASS(wintc_sh_view_desktop_parent_class))
        ->constructed(object);

    // Create the backing desktop view
    //
    GError*            error     = NULL;
    WinTCShextPathInfo path_info = { 0 };

    path_info.base_path =
        g_strdup_printf(
            "file://%s",
            g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP)
        );

    view_desk->view_user_desktop =
        wintc_shext_host_get_view_for_path(
            view_desk->shext_host,
            &path_info,
            &error
        );

    wintc_shext_path_info_free_data(&path_info);

    if (!(view_desk->view_user_desktop))
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    // Link up with desktop view
    //
    g_signal_connect(
        view_desk->view_user_desktop,
        "items-added",
        G_CALLBACK(on_view_user_desktop_items_added),
        view_desk
    );
    g_signal_connect(
        view_desk->view_user_desktop,
        "items-removed",
        G_CALLBACK(on_view_user_desktop_items_removed),
        view_desk
    );
    g_signal_connect(
        view_desk->view_user_desktop,
        "refreshing",
        G_CALLBACK(on_view_user_desktop_refreshing),
        view_desk
    );
}

static void wintc_sh_view_desktop_dispose(
    GObject* object
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(object);

    g_clear_object(&(view_desk->shext_host));
    g_clear_object(&(view_desk->view_user_desktop));

    (G_OBJECT_CLASS(wintc_sh_view_desktop_parent_class))
        ->dispose(object);
}

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

static void wintc_sh_view_desktop_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            view_desk->shext_host = g_value_dup_object(value);
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
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    WINTC_SAFE_REF_CLEAR(error);

    WinTCShextViewItem* item =
        wintc_sh_view_desktop_get_view_item(view_desk, item_hash);

    // If this isn't one of the desktop items, forward to the FS view
    //
    if (!item)
    {
        return wintc_ishext_view_activate_item(
            view_desk->view_user_desktop,
            item_hash,
            path_info,
            error
        );
    }

    // This is ours, deal with it
    //
    if (!(item->priv))
    {
        g_critical("%s", "shell: desk view can't activate item, no target");
        return FALSE;
    }

    path_info->base_path = g_strdup(item->priv);

    return TRUE;
}

static gint wintc_sh_view_desktop_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    gint order_item1 = wintc_sh_view_desktop_get_item_order(item_hash1);
    gint order_item2 = wintc_sh_view_desktop_get_item_order(item_hash2);

    // If they're both FS items, then forward onto the fs view
    //
    if (order_item1 == order_item2 && order_item1 == K_ORDER_FS_ITEM)
    {
        return wintc_ishext_view_compare_items(
            view_desk->view_user_desktop,
            item_hash1,
            item_hash2
        );
    }

    return
        order_item1 < order_item2 ? -1 :
            (order_item1 > order_item2 ? 1 : 0);
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
    WinTCIShextView* view
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    // Aggregate the lists
    //
    return wintc_list_append_list(
        g_hash_table_get_values(S_DESKTOP_MAP),
        wintc_ishext_view_get_items(view_desk->view_user_desktop)
    );
}

static GMenuModel* wintc_sh_view_desktop_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    // Check if this is one of ours
    //
    WinTCShextViewItem* item =
        wintc_sh_view_desktop_get_view_item(view_desk, item_hash);

    if (item)
    {
        g_warning("%s Not Implemented", __func__);
        return NULL;
    }

    // Assume it's for the FS view
    //
    return wintc_ishext_view_get_operations_for_item(
        view_desk->view_user_desktop,
        item_hash
    );
}

static GMenuModel* wintc_sh_view_desktop_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menudesk.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    menu =
        G_MENU_MODEL(
            g_object_ref(
                gtk_builder_get_object(builder, "menu")
            )
        );

    // Populate New submenu
    //
    GMenu* section_new =
        G_MENU(
            gtk_builder_get_object(
                builder,
                "section-new"
            )
        );

    GMenuModel* menu_new = wintc_sh_new_menu_get_menu();

    g_menu_insert_submenu(
        section_new,
        0,
        "New",
        menu_new
    );

    g_object_unref(builder);

    return menu;
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
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh desktop view");

    // Refresh the user desktop view, this will in turn cause the desktop to
    // refresh as well
    //
    wintc_ishext_view_refresh_items(
        view_desk->view_user_desktop
    );
}

static WinTCShextOperation* wintc_sh_view_desktop_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
)
{
    WinTCShViewDesktop* view_desk = WINTC_SH_VIEW_DESKTOP(view);

    // Spawn op
    //
    WinTCShextOperation* ret = NULL;

    if (targets)
    {
        targets =
            wintc_sh_view_desktop_filter_op_targets(
                view_desk,
                targets
            );

        // Check the first item to determine whether it's one of our
        // desktop items or to forward the whole lot to the FS view
        //
        if (
            wintc_sh_view_desktop_get_item_order(
                GPOINTER_TO_UINT(targets->data)
            ) == K_ORDER_FS_ITEM
        )
        {
            return wintc_ishext_view_spawn_operation(
                view_desk->view_user_desktop,
                operation_id,
                targets,
                error
            );
        }
        else
        {
            // FIXME: Implement this
            //
            g_critical(
                "%s",
                "shell: desktop - spawn op not implemented for item"
            );
        }
    
    }
    else
    {
        ret = g_new(WinTCShextOperation, 1);

        ret->view = view;

        switch (operation_id)
        {
            case WINTC_SHEXT_KNOWN_OP_PROPERTIES:
                // FIXME: Handle for view items
                //
                ret->func = shopr_properties;
                break;

            default:
                g_clear_pointer(&ret, (GDestroyNotify) g_free);

                if (WINTC_SHEXT_OP_IS_NEW_OP(operation_id))
                {
                    return wintc_ishext_view_spawn_operation(
                        view_desk->view_user_desktop,
                        operation_id,
                        NULL,
                        error
                    );
                }

                g_critical("%s", "shell: desktop - invalid op");

                break;
        }
    }

    g_clear_list(&targets, NULL);

    return ret;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_desktop_new(
    WinTCShextHost* shext_host
)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DESKTOP,
            "shext-host", shext_host,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gint wintc_sh_view_desktop_get_item_order(
    guint item_hash
)
{
    gint order =
        GPOINTER_TO_INT(
            g_hash_table_lookup(
                S_DESKTOP_ORDER,
                GUINT_TO_POINTER(item_hash)
            )
        );

    if (!order)
    {
        return K_ORDER_FS_ITEM;
    }

    return order;
}

static WinTCShextViewItem* wintc_sh_view_desktop_get_view_item(
    WINTC_UNUSED(WinTCShViewDesktop* view_desk),
    guint item_hash
)
{
    return
        (WinTCShextViewItem*)
        g_hash_table_lookup(
            S_DESKTOP_MAP,
            GUINT_TO_POINTER(item_hash)
        );
}

static GList* wintc_sh_view_desktop_filter_op_targets(
    WINTC_UNUSED(WinTCShViewDesktop* view_desk),
    GList* targets
)
{
    if (!targets)
    {
        return NULL;
    }

    //
    // Targets must be filtered in case there's a mix of desktop items and FS
    // view items
    //
    // This is based on the first item in the view targets list, which SHOULD
    // be the one the user right clicked on in the first place
    //

    // We can make this easy by using the existing 'get_item_order' function
    // because it determines whether the item is ours or not
    //
    gint order =
        wintc_sh_view_desktop_get_item_order(
            GPOINTER_TO_UINT(targets->data)
        );

    if (order == K_ORDER_FS_ITEM)
    {
        // Ensure all other items are FS items
        //
        GList* iter = targets;
        GList* next = NULL;

        while (iter)
        {
            order =
                wintc_sh_view_desktop_get_item_order(
                    GPOINTER_TO_UINT(iter->data)
                );

            next = iter->next;

            if (order != K_ORDER_FS_ITEM)
            {
                targets = g_list_delete_link(targets, iter);
            }

            iter = next;
        }
    }
    else
    {
        // Delete everything other than the first item
        //
        GList* remaining = g_list_remove_link(targets, targets);

        g_list_free(remaining);
    }

    return targets;
}

static void wintc_sh_view_desktop_real_refresh_items(
    WinTCShViewDesktop* view_desk
)
{
    WinTCIShextView* view = WINTC_ISHEXT_VIEW(view_desk);

    _wintc_ishext_view_refreshing(view);

    // Just emit the default items for now
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(S_DESKTOP_MAP);

    update.data = items;
    update.done = FALSE; // The FS view will always follow behind

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

//
// CALLBACKS
//
static gboolean shopr_properties(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(WinTCShextOperation* operation),
    WINTC_UNUSED(GtkWindow* wnd),
    GError** error
)
{
    // FIXME: Clash with WINE
    return wintc_launch_command("desk.cpl", error);
}

static void on_view_user_desktop_items_added(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    // Just forward
    //
    _wintc_ishext_view_items_added(
        WINTC_ISHEXT_VIEW(user_data),
        update
    );
}

static void on_view_user_desktop_items_removed(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    // Just forward
    //
    _wintc_ishext_view_items_removed(
        WINTC_ISHEXT_VIEW(user_data),
        update
    );
}

static void on_view_user_desktop_refreshing(
    WINTC_UNUSED(WinTCIShextView* view),
    gpointer user_data
)
{
    // Only worry about refreshing us
    //
    wintc_sh_view_desktop_real_refresh_items(
        WINTC_SH_VIEW_DESKTOP(user_data)
    );
}
