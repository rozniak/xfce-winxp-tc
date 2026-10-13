#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/actions.h"
#include "../public/fsclipbd.h"
#include "../public/fsop.h"
#include "../public/sound.h"
#include "../public/vwtrash.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_CAN_EMPTY = 1,
    N_PROPERTIES,

    PROP_ICON_NAME
};

enum
{
    SHEXT_CUSTOM_OP_EMPTY = 100,
    SHEXT_CUSTOM_OP_RESTORE
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_trash_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_trash_dispose(
    GObject* object
);
static void wintc_sh_view_trash_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

static gboolean wintc_sh_view_trash_activate_item(
    WinTCIShextView*    view,
    guint               item_data,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_sh_view_trash_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static GList* wintc_sh_view_trash_drag_execute(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_trash_drag_test(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_trash_drop_execute(
    WinTCIShextView*    view,
    GtkWindow*          wnd,
    guint               item_hash,
    const gchar* const* uris,
    gboolean            hint_copy,
    GError**            error
);
static gboolean wintc_sh_view_trash_drop_test(
    WinTCIShextView*    view,
    guint               item_hash,
    const gchar* const* uris
);
static GActionGroup* wintc_sh_view_trash_get_actions(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_trash_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_trash_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_sh_view_trash_get_items(
    WinTCIShextView* view
);
static GMenuModel* wintc_sh_view_trash_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_sh_view_trash_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_sh_view_trash_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_sh_view_trash_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static GMenuModel* wintc_sh_view_trash_get_suggested_actions(
    WinTCIShextView* view,
    guint            item_hash
);
static guint wintc_sh_view_trash_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_sh_view_trash_has_parent(
    WinTCIShextView* view
);
static void wintc_sh_view_trash_refresh_items(
    WinTCIShextView* view
);
static WinTCShextOperation* wintc_sh_view_trash_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);
static void wintc_sh_view_trash_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
);

static void clear_view_item(
    WinTCShextViewItem* item
);

static gchar* wintc_sh_view_trash_build_path_for_view_item(
    WinTCShViewTrash*   view_trash,
    WinTCShextViewItem* item
);
static GList* wintc_sh_view_trash_convert_list_hashes(
    WinTCShViewTrash* view_trash,
    GList*            list_items
);
static guint wintc_sh_view_trash_get_unique_item_hash(
    WinTCShViewTrash* view_trash,
    const gchar*      name
);
static WinTCShextViewItem* wintc_sh_view_trash_get_view_item(
    WinTCShViewTrash* view_trash,
    guint             item_hash
);

void action_cut(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
void action_delete(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
void action_empty(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
void action_paste(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
void action_restore(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void on_file_monitor_changed(
    GFileMonitor*     self,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);
static void on_fs_operation_done(
    WinTCShFSOperation* self,
    gpointer            user_data
);
static void on_fs_operation_done_with_sound(
    WinTCShFSOperation* self,
    gpointer            user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_sh_view_trash_properties[N_PROPERTIES] = { 0 };

static GActionEntry S_TRASH_ACTIONS[] = {
    {
        .name           = "cut",
        .activate       = action_cut,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "delete",
        .activate       = action_delete,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "empty",
        .activate       = action_empty,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "paste",
        .activate       = action_paste,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "restore",
        .activate       = action_restore,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShViewTrash
{
    GObject __parent__;

    // State
    //
    GFileMonitor* file_monitor;
    GFile*        file_trash;
    GHashTable*   map_entries;

    WinTCShFSClipboard* fs_clipboard;
    GHashTable* map_actions_to_targets;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewTrash,
    wintc_sh_view_trash,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_trash_ishext_view_interface_init
    )
)

WINTC_SH_DEFINE_CB_VW_ACTIONS_CLEANUP(
    WinTCShViewTrash,
    vw_trash,
    map_actions_to_targets
)

static void wintc_sh_view_trash_class_init(
    WinTCShViewTrashClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_sh_view_trash_dispose;
    object_class->get_property = wintc_sh_view_trash_get_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );

    wintc_sh_view_trash_properties[PROP_CAN_EMPTY] =
        g_param_spec_boolean(
            "can-empty",
            "CanEmpty",
            "Determines whether the Recycle Bin can be emptied.",
            FALSE,
            G_PARAM_READABLE
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_sh_view_trash_properties
    );
}

static void wintc_sh_view_trash_init(
    WinTCShViewTrash* self
)
{
    GError* error = NULL;

    self->file_trash   = g_file_new_for_uri("trash:///");
    self->fs_clipboard = wintc_sh_fs_clipboard_new();

    self->file_monitor =
        g_file_monitor_directory(
            self->file_trash,
            0,
            NULL,
            &error
        );

    if (self->file_monitor)
    {
        g_signal_connect(
            self->file_monitor,
            "changed",
            G_CALLBACK(on_file_monitor_changed),
            self
        );
    }

    WINTC_SH_INIT_VW_ACTIONS(self, map_actions_to_targets);
}

static void wintc_sh_view_trash_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_sh_view_trash_activate_item;
    iface->compare_items           = wintc_sh_view_trash_compare_items;
    iface->drag_execute            = wintc_sh_view_trash_drag_execute;
    iface->drag_test               = wintc_sh_view_trash_drag_test;
    iface->drop_execute            = wintc_sh_view_trash_drop_execute;
    iface->drop_test               = wintc_sh_view_trash_drop_test;
    iface->get_actions             = wintc_sh_view_trash_get_actions;
    iface->get_display_name        = wintc_sh_view_trash_get_display_name;
    iface->get_icon_name           = wintc_sh_view_trash_get_icon_name;
    iface->get_items               = wintc_sh_view_trash_get_items;
    iface->get_operations_for_item =
        wintc_sh_view_trash_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_sh_view_trash_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_trash_get_parent_path;
    iface->get_path                = wintc_sh_view_trash_get_path;
    iface->get_suggested_actions   = wintc_sh_view_trash_get_suggested_actions;
    iface->get_unique_hash         = wintc_sh_view_trash_get_unique_hash;
    iface->has_parent              = wintc_sh_view_trash_has_parent;
    iface->refresh_items           = wintc_sh_view_trash_refresh_items;
    iface->spawn_operation         = wintc_sh_view_trash_spawn_operation;
    iface->update_actions          = wintc_sh_view_trash_update_actions;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_trash_dispose(
    GObject* object
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(object);

    g_clear_object(&(view_trash->file_monitor));
    g_clear_object(&(view_trash->file_trash));
    g_clear_object(&(view_trash->fs_clipboard));

    WINTC_SH_DESTROY_VW_ACTIONS_MAP(view_trash, map_actions_to_targets);

    if (view_trash->map_entries)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_trash->map_entries))
        );
    }

    (G_OBJECT_CLASS(wintc_sh_view_trash_parent_class))
        ->dispose(object);
}

static void wintc_sh_view_trash_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            g_value_set_string(
                value,
                wintc_sh_view_trash_get_icon_name(
                    WINTC_ISHEXT_VIEW(view_trash)
                )
            );
            break;

        case PROP_CAN_EMPTY:
            g_value_set_boolean(
                value,
                view_trash->map_entries &&
                g_hash_table_size(view_trash->map_entries)
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
static gboolean wintc_sh_view_trash_activate_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_data),
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    GError** error
)
{
    // FIXME: Implement this
    //
    g_set_error(
        error,
        wintc_general_error_quark(),
        WINTC_GENERAL_ERROR_NOTIMPL,
        "Method not implemented: %s",
        __func__
    );

    return FALSE;
}

static gint wintc_sh_view_trash_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    return wintc_shext_view_item_compare_by_fs_order(
        wintc_sh_view_trash_get_view_item(view_trash, item_hash1),
        wintc_sh_view_trash_get_view_item(view_trash, item_hash2)
    );
}

static GList* wintc_sh_view_trash_drag_execute(
    WinTCIShextView* view,
    GList*           item_hashes,
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    GList* list_uris = NULL;

    for (GList* iter = item_hashes; iter; iter = iter->next)
    {
        WinTCShextViewItem* item =
            wintc_sh_view_trash_get_view_item(
                view_trash,
                GPOINTER_TO_UINT(iter->data)
            );

        list_uris =
            g_list_prepend(
                list_uris,
                wintc_sh_view_trash_build_path_for_view_item(
                    view_trash,
                    item
                )
            );
    }

    return g_list_reverse(list_uris);
}

static gboolean wintc_sh_view_trash_drag_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(GList*              item_hashes),
    WINTC_UNUSED(WinTCShextDndTarget target)
)
{
    return TRUE;
}

static gboolean wintc_sh_view_trash_drop_execute(
    WINTC_UNUSED(WinTCIShextView* view),
    GtkWindow*          wnd,
    WINTC_UNUSED(guint item_hash),
    const gchar* const* uris,
    WINTC_UNUSED(gboolean hint_copy),
    WINTC_UNUSED(GError** error)
)
{
    // Get the items we're to trash
    //
    GList* sources = wintc_list_new_from_const_strv(uris);

    // Do the trash operation
    //
    WinTCShFSOperation* op =
        wintc_sh_fs_operation_new(
            sources,
            NULL,
            WINTC_SH_FS_OPERATION_TRASH
        );

    g_signal_connect(
        op,
        "done",
        G_CALLBACK(on_fs_operation_done),
        sources
    );

    wintc_sh_fs_operation_do(op, wnd);

    return TRUE;
}

static gboolean wintc_sh_view_trash_drop_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(const gchar* const* uris)
)
{
    // Accept all drops - treat them all as the view
    //
    return TRUE;
}

static GActionGroup* wintc_sh_view_trash_get_actions(
    WinTCIShextView* view
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    GSimpleActionGroup* actions = g_simple_action_group_new();

    WINTC_SH_CREATE_VW_ACTIONS_GROUP(
        view_trash,
        map_actions_to_targets,
        actions,
        S_TRASH_ACTIONS
    );

    // Bind 'empty' action
    //
    GAction* action_empty =
        g_action_map_lookup_action(
            G_ACTION_MAP(actions),
            "empty"
        );

    g_object_bind_property(
        G_OBJECT(view_trash),
        "can-empty",
        G_OBJECT(action_empty),
        "enabled",
        G_BINDING_SYNC_CREATE
    );

    return G_ACTION_GROUP(actions);
}

static const gchar* wintc_sh_view_trash_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Use shlang
    //
    return "Recycle Bin";
}

static const gchar* wintc_sh_view_trash_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "user-trash";
}

static GList* wintc_sh_view_trash_get_items(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    return g_hash_table_get_values(view_trash->map_entries);
}

static GMenuModel* wintc_sh_view_trash_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menurblf.ui"
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

static GMenuModel* wintc_sh_view_trash_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menurb.ui"
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

static void wintc_sh_view_trash_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
        );
}

static void wintc_sh_view_trash_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_RECYCLEBIN)
        );
}

static GMenuModel* wintc_sh_view_trash_get_suggested_actions(
    WINTC_UNUSED(WinTCIShextView* view),
    guint item_hash
)
{
    if (item_hash)
    {
        g_critical(
            "%s",
            "shell: vwtrash suggested actions for item not implemented"
        );

        return NULL;
    }

    // Construct the suggested actions menu UI
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource("/uk/oddmatics/wintc/shell/amtrshvw.ui");

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

static guint wintc_sh_view_trash_get_unique_hash(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return g_str_hash(wintc_sh_get_place_path(WINTC_SH_PLACE_RECYCLEBIN));
}

static gboolean wintc_sh_view_trash_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_sh_view_trash_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh trash view");

    _wintc_ishext_view_refreshing(view);

    if (view_trash->map_entries)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_trash->map_entries))
        );
    }

    view_trash->map_entries =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) clear_view_item
        );

    // Enumerate the first level children in the bin
    //
    GError*          error   = NULL;
    GFileEnumerator* fs_enum =
        g_file_enumerate_children(
            view_trash->file_trash,
            G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
            G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
            NULL,
            &error
        );


    if (!fs_enum)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    for (
        GFileInfo* info = g_file_enumerator_next_file(fs_enum, NULL, &error);
        info;
        info = g_file_enumerator_next_file(fs_enum, NULL, &error)
    )
    {
        const gchar* item_name = g_file_info_get_name(info);

        gboolean            is_dir;
        WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

        is_dir = g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY;

        item->display_name = g_strdup(item_name);
        item->icon_name    = is_dir ? "inode-directory" : "empty";
        item->is_leaf      = TRUE;
        item->hash         = wintc_sh_view_trash_get_unique_item_hash(
                                 view_trash,
                                 item_name
                             );

        g_hash_table_insert(
            view_trash->map_entries,
            GUINT_TO_POINTER(item->hash),
            item
        );

        g_object_unref(info);
    }

    g_file_enumerator_close(fs_enum, NULL, NULL);
    g_object_unref(fs_enum);

    if (error)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    // Provide update
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(view_trash->map_entries);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);

    g_object_notify_by_pspec(
        G_OBJECT(view_trash),
        wintc_sh_view_trash_properties[PROP_CAN_EMPTY]
    );
}

static WinTCShextOperation* wintc_sh_view_trash_spawn_operation(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(gint operation_id),
    GList*           targets,
    GError**         error
)
{
    // Spawn op
    //
    switch (operation_id)
    {
        default:
            g_set_error(
                error,
                wintc_general_error_quark(),
                WINTC_GENERAL_ERROR_NOTIMPL,
                "Sorry! Operation not implemented: %d",
                operation_id
            );

            break;
    }

    g_clear_list(&targets, NULL);

    return NULL;
}

static void wintc_sh_view_trash_update_actions(
    WinTCIShextView* view,
    GActionGroup*    actions,
    GList*           items
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(view);

    WINTC_SH_UPDATE_VW_ACTIONS_GROUP(
        view_trash,
        map_actions_to_targets,
        actions,
        items
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_trash_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_TRASH,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void clear_view_item(
    WinTCShextViewItem* item
)
{
    g_free(item->display_name);
    g_free(item);
}

static gchar* wintc_sh_view_trash_build_path_for_view_item(
    WINTC_UNUSED(WinTCShViewTrash* view_trash),
    WinTCShextViewItem* item
)
{
    return g_strdup_printf("trash:///%s", item->display_name);
}

static GList* wintc_sh_view_trash_convert_list_hashes(
    WinTCShViewTrash* view_trash,
    GList*            list_items
)
{
    for (GList* iter = list_items; iter; iter = iter->next)
    {
        WinTCShextViewItem* item =
            wintc_sh_view_trash_get_view_item(
                view_trash,
                GPOINTER_TO_UINT(iter->data)
            );

        iter->data =
            wintc_sh_view_trash_build_path_for_view_item(
                view_trash,
                item
            );
    }

    return list_items;
}

static guint wintc_sh_view_trash_get_unique_item_hash(
    WINTC_UNUSED(WinTCShViewTrash* view_trash),
    const gchar* name
)
{
    guint hash1 = g_str_hash(
                      wintc_sh_get_place_path(WINTC_SH_PLACE_RECYCLEBIN)
                  );
    guint hash2 = g_str_hash(name);

    return hash1 * 33 + hash2;
}

static WinTCShextViewItem* wintc_sh_view_trash_get_view_item(
    WinTCShViewTrash* view_trash,
    guint             item_hash
)
{
    return
        (WinTCShextViewItem*)
        g_hash_table_lookup(
            view_trash->map_entries,
            GUINT_TO_POINTER(item_hash)
        );
}

//
// CALLBACKS
//
void action_cut(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx        = (WinTCShViewActions*) user_data;
    WinTCShViewTrash*   view_trash = WINTC_SH_VIEW_TRASH(ctx->view);

    GError* error        = NULL;
    GList*  list_targets =
        wintc_sh_view_trash_convert_list_hashes(
            view_trash,
            g_hash_table_lookup(
                view_trash->map_actions_to_targets,
                ctx->action_group
            )
        );

    if (
        !wintc_sh_fs_clipboard_copymove(
            view_trash->fs_clipboard,
            list_targets,
            TRUE,
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error, NULL); // wnd
    }
}

void action_delete(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx        = (WinTCShViewActions*) user_data;
    WinTCShViewTrash*   view_trash = WINTC_SH_VIEW_TRASH(ctx->view);

    GList*              list_targets;
    WinTCShFSOperation* op;

    list_targets =
        wintc_sh_view_trash_convert_list_hashes(
            view_trash,
            g_hash_table_lookup(
                view_trash->map_actions_to_targets,
                ctx->action_group
            )
        );

    op =
        wintc_sh_fs_operation_new(
            list_targets,
            NULL,
            WINTC_SH_FS_OPERATION_DELETE
        );

    g_signal_connect(
        op,
        "done",
        G_CALLBACK(on_fs_operation_done),
        list_targets
    );

    wintc_sh_fs_operation_do(op, NULL); // wnd
}

void action_empty(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx        = (WinTCShViewActions*) user_data;
    WinTCShViewTrash*   view_trash = WINTC_SH_VIEW_TRASH(ctx->view);

    // Convert all the entries into paths for deletion
    //
    GList* list_targets =
        g_hash_table_get_keys(view_trash->map_entries);

    wintc_sh_view_trash_convert_list_hashes(
        view_trash,
        list_targets
    );

    // Set up and execute the delete operation
    //
    WinTCShFSOperation* op =
        wintc_sh_fs_operation_new(
            list_targets,
            NULL,
            WINTC_SH_FS_OPERATION_DELETE
        );

    g_signal_connect(
        op,
        "done",
        G_CALLBACK(on_fs_operation_done_with_sound),
        list_targets
    );

    wintc_sh_fs_operation_do(op, NULL); // wnd
}

void action_paste(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx        = (WinTCShViewActions*) user_data;
    WinTCShViewTrash*   view_trash = WINTC_SH_VIEW_TRASH(ctx->view);

    GError* error = NULL;

    if (
        !wintc_sh_fs_clipboard_paste(
            view_trash->fs_clipboard,
            "trash:///",
            NULL, // wnd
            &error
        )
    )
    {
        wintc_display_error_and_clear(&error, NULL); // wnd
    }
}

void action_restore(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant* parameter),
    gpointer user_data
)
{
    WinTCShViewActions* ctx        = (WinTCShViewActions*) user_data;
    WinTCShViewTrash*   view_trash = WINTC_SH_VIEW_TRASH(ctx->view);

    //
    // FIXME: FSOP cannot do a restore, since each src->dest is different so
    //        we're just doing a simple set of moves here, single threaded
    //
    GError*  error   = NULL;
    GList*   list_targets;
    gboolean success = TRUE;

    list_targets =
        wintc_sh_view_trash_convert_list_hashes(
            view_trash,
            g_hash_table_lookup(
                view_trash->map_actions_to_targets,
                ctx->action_group
            )
        );

    for (GList* iter = list_targets; iter; iter = iter->next)
    {
        const gchar* uri = (gchar*) iter->data;

        GFile*     file = g_file_new_for_uri(uri);
        GFileInfo* info = g_file_query_info(
                              file,
                              G_FILE_ATTRIBUTE_TRASH_ORIG_PATH,
                              G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                              NULL,
                              NULL
                          );

        const gchar* orig_path = g_file_info_get_attribute_byte_string(
                                     info,
                                     G_FILE_ATTRIBUTE_TRASH_ORIG_PATH
                                 );
        GFile*       dest      = g_file_new_for_path(orig_path);

        success =
            g_file_move(
                file,
                dest,
                G_FILE_COPY_NOFOLLOW_SYMLINKS | G_FILE_COPY_ALL_METADATA,
                NULL,
                NULL,
                NULL,
                &error
            );

        g_object_unref(dest);
        g_object_unref(info);
        g_object_unref(file);

        if (!success)
        {
            wintc_display_error_and_clear(&error, NULL);
            break;
        }
    }

    g_list_foreach(list_targets, (GFunc) wintc_free_cb, NULL);
}

static void on_file_monitor_changed(
    WINTC_UNUSED(GFileMonitor* self),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCShViewTrash* view_trash = WINTC_SH_VIEW_TRASH(user_data);

    GList*                    data   = NULL;
    GFileInfo*                info;
    gboolean                  is_dir;
    WinTCShextViewItem*       item;
    gchar*                    name   = g_file_get_basename(file);
    WinTCShextViewItemsUpdate update = { 0 };

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            info =
                g_file_query_info(
                    file,
                    G_FILE_ATTRIBUTE_STANDARD_TYPE,
                    G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                    NULL,
                    NULL
                );

            is_dir = g_file_info_get_file_type(info) == G_FILE_TYPE_DIRECTORY;

            WINTC_LOG_DEBUG("shell: trash monitor - %s created", name);

            item = g_new(WinTCShextViewItem, 1);

            item->display_name = g_steal_pointer(&name);
            item->icon_name    = is_dir ? "inode-directory" : "empty";
            item->is_leaf      = TRUE;
            item->hash         = wintc_sh_view_trash_get_unique_item_hash(
                                     view_trash,
                                     item->display_name
                                 );

            g_hash_table_insert(
                view_trash->map_entries,
                GUINT_TO_POINTER(item->hash),
                item
            );

            // Issue update
            //
            data = g_list_prepend(data, item);

            update.data = data;
            update.done = TRUE;

            _wintc_ishext_view_items_added(
                WINTC_ISHEXT_VIEW(view_trash),
                &update
            );

            g_object_unref(info);

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
        {
            guint item_hash =
                wintc_sh_view_trash_get_unique_item_hash(
                    view_trash,
                    name
                );

            WINTC_LOG_DEBUG("shell: trash monitor - %s deleted", name);

            g_hash_table_remove(
                view_trash->map_entries,
                GUINT_TO_POINTER(item_hash)
            );

            // Issue update
            //
            data =
                g_list_prepend(
                    data,
                    GUINT_TO_POINTER(item_hash)
                );

            update.data = data;
            update.done = TRUE;

            _wintc_ishext_view_items_removed(
                WINTC_ISHEXT_VIEW(view_trash),
                &update
            );

            break;
        }

        default: break;
    }

    g_list_free(data);
    g_free(name);

    g_object_notify_by_pspec(
        G_OBJECT(view_trash),
        wintc_sh_view_trash_properties[PROP_CAN_EMPTY]
    );
}

static void on_fs_operation_done(
    WinTCShFSOperation* self,
    gpointer            user_data
)
{
    g_list_foreach((GList*) user_data, (GFunc) wintc_free_cb, NULL);
    g_object_unref(self);
}

static void on_fs_operation_done_with_sound(
    WinTCShFSOperation* self,
    gpointer            user_data
)
{
    wintc_sh_play_sound(WINTC_SHELL_SND_EMPTYBIN);
    on_fs_operation_done(self, user_data);
}
