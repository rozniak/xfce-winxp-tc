#include <gio/gio.h>
#include <glib.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/fsclipbd.h"
#include "../public/fsop.h"
#include "../public/vwfs.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SHEXT_HOST = 1,
    PROP_PATH,
    PROP_ICON_NAME
};

enum
{
    WINTC_SH_VIEW_FS_OP_NEW_FOLDER   = 80,
    WINTC_SH_VIEW_FS_OP_NEW_SHORTCUT = 100
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCShViewFSNewTemplate
{
    gchar* filename;
    gchar* name;
} WinTCShViewFSNewTemplate;

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_fs_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_fs_dispose(
    GObject* object
);
static void wintc_sh_view_fs_finalize(
    GObject* object
);
static void wintc_sh_view_fs_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_view_fs_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_sh_view_fs_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_sh_view_fs_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static const gchar* wintc_sh_view_fs_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_fs_get_icon_name(
    WinTCIShextView* view
);
static GList* wintc_sh_view_fs_get_items(
    WinTCIShextView* view
);
static GMenuModel* wintc_sh_view_fs_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_sh_view_fs_get_operations_for_view(
    WinTCIShextView* view
);
static void wintc_sh_view_fs_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_sh_view_fs_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static guint wintc_sh_view_fs_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_sh_view_fs_has_parent(
    WinTCIShextView* view
);
static void wintc_sh_view_fs_refresh_items(
    WinTCIShextView* view
);
static WinTCShextOperation* wintc_sh_view_fs_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    GError**         error
);

static void clear_new_template(
    WinTCShViewFSNewTemplate* template
);
static void clear_view_item(
    WinTCShextViewItem* item
);

static gchar* get_file_mime_icon(
    GFile* file
);
static gboolean real_activate_item(
    WinTCShViewFS*      view_fs,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static gchar* wintc_sh_view_fs_build_path_for_view_item(
    WinTCShViewFS*      view_fs,
    WinTCShextViewItem* item
);
static GList* wintc_sh_view_fs_convert_list_hashes(
    WinTCShViewFS* view_fs,
    GList*         list_items
);
static WinTCShextViewItem* wintc_sh_view_fs_get_view_item(
    WinTCShViewFS* view_fs,
    guint          item_hash
);
static void wintc_sh_view_fs_update_new_templates(
    WinTCShViewFS* view_fs
);

static gboolean shopr_delete(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
);
static gboolean shopr_new(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
);
static gboolean shopr_open(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
);
static gboolean shopr_paste(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    GError**             error
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
    WINTC_UNUSED(gpointer user_data)
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCShViewFSClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewFS
{
    GObject __parent__;

    // FS state
    //
    gboolean is_new; // Track if this view has ever been refreshed before
    gchar*   parent_path;
    gchar*   path;

    GFileMonitor* fs_monitor;
    GHashTable*   fs_map_entries;

    WinTCShFSClipboard* fs_clipboard;
    WinTCShextHost*     shext_host;

    GList* list_new_templates;
    guint  next_new_hash; // For flagging a view item we just made as new
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewFS,
    wintc_sh_view_fs,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_fs_ishext_view_interface_init
    )
)

static void wintc_sh_view_fs_class_init(
    WinTCShViewFSClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_sh_view_fs_dispose;
    object_class->finalize     = wintc_sh_view_fs_finalize;
    object_class->get_property = wintc_sh_view_fs_get_property;
    object_class->set_property = wintc_sh_view_fs_set_property;

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
            "The shell extension host.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_PATH,
        g_param_spec_string(
            "path",
            "Path",
            "The path to open in the view.",
            NULL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        )
    );
}

static void wintc_sh_view_fs_init(
    WinTCShViewFS* self
)
{
    self->is_new       = TRUE;
    self->fs_clipboard = wintc_sh_fs_clipboard_new();
}

static void wintc_sh_view_fs_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_sh_view_fs_activate_item;
    iface->compare_items           = wintc_sh_view_fs_compare_items;
    iface->get_display_name        = wintc_sh_view_fs_get_display_name;
    iface->get_icon_name           = wintc_sh_view_fs_get_icon_name;
    iface->get_items               = wintc_sh_view_fs_get_items;
    iface->get_operations_for_item = wintc_sh_view_fs_get_operations_for_item;
    iface->get_operations_for_view = wintc_sh_view_fs_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_fs_get_parent_path;
    iface->get_path                = wintc_sh_view_fs_get_path;
    iface->get_unique_hash         = wintc_sh_view_fs_get_unique_hash;
    iface->has_parent              = wintc_sh_view_fs_has_parent;
    iface->refresh_items           = wintc_sh_view_fs_refresh_items;
    iface->spawn_operation         = wintc_sh_view_fs_spawn_operation;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_fs_dispose(
    GObject* object
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(object);

    g_clear_object(&(view_fs->fs_clipboard));
    g_clear_object(&(view_fs->fs_monitor));
    g_clear_object(&(view_fs->shext_host));

    g_clear_list(
        &(view_fs->list_new_templates),
        (GDestroyNotify) clear_new_template
    );

    (G_OBJECT_CLASS(wintc_sh_view_fs_parent_class))->dispose(object);
}

static void wintc_sh_view_fs_finalize(
    GObject* object
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(object);

    g_free(view_fs->parent_path);
    g_free(view_fs->path);

    if (view_fs->fs_map_entries)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_fs->fs_map_entries))
        );
    }

    (G_OBJECT_CLASS(wintc_sh_view_fs_parent_class))->finalize(object);
}

static void wintc_sh_view_fs_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCIShextView* view    = WINTC_ISHEXT_VIEW(object);
    WinTCShViewFS*   view_fs = WINTC_SH_VIEW_FS(object);

    switch (prop_id)
    {
        case PROP_PATH:
            g_value_set_string(value, view_fs->path);
            break;

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

static void wintc_sh_view_fs_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShViewFS* view = WINTC_SH_VIEW_FS(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            view->shext_host = g_value_dup_object(value);
            break;

        case PROP_PATH:
        {
            const gchar* raw_path = g_value_get_string(value);
            gint         path_len = g_utf8_strlen(raw_path, -1);

            // Strip off trailing /, unless this is literally /
            //
            if (path_len > 1 && g_str_has_suffix(raw_path, "/"))
            {
                view->path =
                    g_utf8_substring(
                        raw_path,
                        0,
                        g_utf8_strlen(raw_path, -1) - 1
                    );
            }
            else
            {
                view->path = g_strdup(raw_path);
            }

            // Create parent path string
            //
            if (g_strcmp0(view->path, "/") != 0)
            {
                view->parent_path = g_path_get_dirname(view->path);
            }

            break;
        }

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// INTERFACE METHODS (WinTCIShextView)
//
static gboolean wintc_sh_view_fs_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WINTC_SAFE_REF_CLEAR(error);

    // Retrieve the item itself
    //
    WinTCShextViewItem* item =
        wintc_sh_view_fs_get_view_item(view_fs, item_hash);

    if (!item)
    {
        g_critical(
            "%s",
            "shell: fs - attempt to activate non-existent item"
        );
        return FALSE;
    }

    return real_activate_item(
        view_fs,
        item,
        path_info,
        error
    );
}

static gint wintc_sh_view_fs_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    return wintc_shext_view_item_compare_by_fs_order(
        wintc_sh_view_fs_get_view_item(view_fs, item_hash1),
        wintc_sh_view_fs_get_view_item(view_fs, item_hash2)
    );
}

static const gchar* wintc_sh_view_fs_get_display_name(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    if (g_strcmp0(view_fs->path, "/") == 0)
    {
        return view_fs->path;
    }

    // FIXME: This could be broken if the path itself contains an escaped dir
    //        separator, cba for now
    //
    return g_strrstr(view_fs->path, G_DIR_SEPARATOR_S) + 1;
}

static const gchar* wintc_sh_view_fs_get_icon_name(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    if (g_strcmp0(view_fs->path, "/") == 0)
    {
        return "drive-harddisk";
    }

    return "inode-directory";
}

static GList* wintc_sh_view_fs_get_items(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    if (view_fs->is_new)
    {
        wintc_sh_view_fs_refresh_items(view);
        return NULL;
    }

    return g_hash_table_get_values(view_fs->fs_map_entries);
}

static GMenuModel* wintc_sh_view_fs_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
)
{
    // Retrieve the view item itself
    //
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WinTCShextViewItem* view_item =
        wintc_sh_view_fs_get_view_item(view_fs, item_hash);

    if (!view_item)
    {
        g_critical("%s", "shell: fs - no such item for menu");
        return NULL;
    }

    // FIXME: Extremely simplistic for now, does not cover shell extensions or
    //        anything!
    //
    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            view_item->is_leaf ?
              "/uk/oddmatics/wintc/shell/menufslf.ui" :
              "/uk/oddmatics/wintc/shell/menufsvw.ui"
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

static GMenuModel* wintc_sh_view_fs_get_operations_for_view(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    GtkBuilder* builder;
    GMenuModel* menu;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/shell/menufs.ui"
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
    GMenu* section_new_templates =
        G_MENU(
            gtk_builder_get_object(
                builder,
                "section-new-templates"
            )
        );

    gint view_op_id = WINTC_SHEXT_OP_NEW + 1;

    wintc_sh_view_fs_update_new_templates(view_fs);

    for (GList* iter = view_fs->list_new_templates; iter; iter = iter->next)
    {
        WinTCShViewFSNewTemplate* template =
            (WinTCShViewFSNewTemplate*) iter->data;

        // Cap off menu items
        //
        if (!WINTC_SHEXT_OP_IS_NEW_OP(view_op_id))
        {
            break;
        }

        // Create the menu item
        //
        GIcon*     icon      = g_themed_icon_new(template->filename);
        GMenuItem* menu_item = g_menu_item_new(NULL, NULL);

        g_menu_item_set_label(menu_item, template->name);
        g_menu_item_set_icon(menu_item, icon);

        g_menu_item_set_action_and_target(
            menu_item,
            "control.view-op",
            "i",
            view_op_id
        );

        g_menu_append_item(
            section_new_templates,
            menu_item
        );

        view_op_id++;

        g_object_unref(icon);
        g_object_unref(menu_item);
    }

    g_object_unref(builder);

    return menu;
}

static void wintc_sh_view_fs_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    if (view_fs->parent_path)
    {
        path_info->base_path =
            g_strdup_printf("file://%s", view_fs->parent_path);
    }
    else
    {
        path_info->base_path =
            g_strdup(
                wintc_sh_get_place_path(
                    WINTC_SH_PLACE_DRIVES
                )
            );
    }
}

static void wintc_sh_view_fs_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    path_info->base_path =
        g_strdup_printf("file://%s", view_fs->path);
}

static guint wintc_sh_view_fs_get_unique_hash(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    return g_str_hash(view_fs->path);
}

static gboolean wintc_sh_view_fs_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_sh_view_fs_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh fs view");

    view_fs->is_new = FALSE;

    _wintc_ishext_view_refreshing(view);

    if (view_fs->fs_map_entries)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_fs->fs_map_entries))
        );
    }

    // FIXME: Error handling (no way of passing to caller)
    //
    GList* entries =
        wintc_sh_fs_get_names_as_list(
            view_fs->path,
            FALSE,
            0,
            FALSE,
            NULL
        );

    // Spawn file monitor if needed
    //
    if (!view_fs->fs_monitor)
    {
        GFile* this_dir = g_file_new_for_path(view_fs->path);

        view_fs->fs_monitor =
            g_file_monitor_directory(
                this_dir,
                0,
                NULL,
                NULL
            );

        if (view_fs->fs_monitor)
        {
            g_signal_connect(
                view_fs->fs_monitor,
                "changed",
                G_CALLBACK(on_file_monitor_changed),
                view_fs
            );
        }

        g_object_unref(this_dir);
    }

    // Enumerate the entries now
    //
    gchar* entry_path;

    view_fs->fs_map_entries =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) clear_view_item
        );

    for (GList* iter = entries; iter; iter = iter->next)
    {
        GFile*              file;
        gboolean            is_dir;
        WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

        entry_path =
            g_build_path(
                G_DIR_SEPARATOR_S,
                view_fs->path,
                (gchar*) iter->data,
                NULL
            );

        file   = g_file_new_for_path(entry_path);
        is_dir = g_file_test(entry_path, G_FILE_TEST_IS_DIR);

        item->display_name = (gchar*) g_steal_pointer(&(iter->data));
        item->icon_name    = is_dir ?
                                 g_strdup("inode-directory") :
                                 get_file_mime_icon(file);
        item->is_leaf      = !is_dir;
        item->hash         = g_str_hash(entry_path);

        g_free(entry_path);
        g_object_unref(file);

        g_hash_table_insert(
            view_fs->fs_map_entries,
            GUINT_TO_POINTER(item->hash),
            item
        );
    }

    g_list_free(entries);

    // Provide update
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(view_fs->fs_map_entries);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

static WinTCShextOperation* wintc_sh_view_fs_spawn_operation(
    WinTCIShextView* view,
    gint             operation_id,
    GList*           targets,
    WINTC_UNUSED(GError**         error)
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    // Spawn op
    //
    WinTCShextOperation* ret = g_new(WinTCShextOperation, 1);

    switch (operation_id)
    {
        case WINTC_SHEXT_KNOWN_OP_OPEN:
            ret->func = shopr_open;
            ret->priv = g_steal_pointer(&targets);
            break;

        case WINTC_SHEXT_KNOWN_OP_PASTE:
            ret->func = shopr_paste;
            break;

        case WINTC_SHEXT_KNOWN_OP_DELETE:
            ret->func = shopr_delete;
            ret->priv = wintc_sh_view_fs_convert_list_hashes(
                            view_fs,
                            g_steal_pointer(&targets)
                        );
            break;

        default:
            // Could be a NEW operation...
            //
            if (WINTC_SHEXT_OP_IS_NEW_OP(operation_id))
            {
                ret->func = shopr_new;
                ret->priv = GINT_TO_POINTER(operation_id);
            }
            else
            {
                g_free(g_steal_pointer(&ret));
                g_critical("%s", "shell: fs - invalid op");
            }

            break;
    }

    g_clear_list(&targets, NULL);

    return ret;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_fs_new(
    WinTCShextHost* shext_host,
    const gchar*    path
)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_FS,
            "path",       path,
            "shext-host", shext_host,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void clear_new_template(
    WinTCShViewFSNewTemplate* template
)
{
    g_free(template->filename);
    g_free(template->name);
    g_free(template);
}

static void clear_view_item(
    WinTCShextViewItem* item
)
{
    g_free(item->display_name);
    g_free(item->icon_name);
    g_free(item);
}

static gchar* get_file_mime_icon(
    GFile* file
)
{
    GFileInfo* file_info =
        g_file_query_info(
            file,
            G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
            G_FILE_QUERY_INFO_NONE,
            NULL,
            NULL
        );

    if (!file_info)
    {
        return g_strdup("empty");
    }

    // Use Gio to look up the icon...
    //
    const gchar* content_type = g_file_info_get_content_type(file_info);
    gchar*       found_icon   = NULL;

    if (content_type)
    {
        GIcon*        icon       = g_content_type_get_icon(content_type);
        GtkIconTheme* icon_theme = gtk_icon_theme_get_default();

        // We good with this icon?
        //
        if (icon && G_IS_THEMED_ICON(icon))
        {
            const gchar* const* icon_names =
                g_themed_icon_get_names(G_THEMED_ICON(icon));

            for (gint i = 0; icon_names[i] != NULL; i++)
            {
                if (
                    gtk_icon_theme_has_icon(
                        icon_theme,
                        icon_names[i]
                    )
                )
                {
                    found_icon = g_strdup(icon_names[i]);
                    break;
                }
            }
        }

        g_clear_object(&icon);
    }

    if (!found_icon)
    {
        found_icon = g_strdup("empty");
    }

    g_object_unref(file_info);

    return found_icon;
}

static gboolean real_activate_item(
    WinTCShViewFS*      view_fs,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    // If the target is a dir or has a shell extension then set that as
    // the target path
    //
    gchar* next_path;
    gchar* target_path = NULL;

    next_path =
        wintc_sh_view_fs_build_path_for_view_item(
            view_fs,
            item
        );

    if (!(item->is_leaf))
    {
        target_path = g_strdup_printf("file://%s", next_path);
    }
    else
    {
        gchar* mime_type = wintc_query_mime_for_file(next_path, error);

        if (!mime_type)
        {
            g_free(next_path);
            return FALSE;
        }

        if (wintc_shext_host_has_view_for_mime(view_fs->shext_host, mime_type))
        {
            target_path = g_strdup_printf("file://%s", next_path);
        }
    }

    // If there is a target path, then either navigate there, or open a new
    // window to navigate there
    //
    // Otherwise just run the command
    //
    gboolean success = TRUE;

    if (target_path)
    {
        if (path_info)
        {
            path_info->base_path = target_path;
        }
        else
        {
            success = wintc_launch_command(target_path, error);

            g_free(target_path);
        }
    }
    else
    {
        success = wintc_launch_command(next_path, error);
    }

    g_free(next_path);

    return success;
}

static gchar* wintc_sh_view_fs_build_path_for_view_item(
    WinTCShViewFS*      view_fs,
    WinTCShextViewItem* item
)
{
    return g_build_path(
        G_DIR_SEPARATOR_S,
        view_fs->path,
        item->display_name,
        NULL
    );
}

static GList* wintc_sh_view_fs_convert_list_hashes(
    WinTCShViewFS* view_fs,
    GList*         list_items
)
{
    for (GList* iter = list_items; iter; iter = iter->next)
    {
        WinTCShextViewItem* item =
            wintc_sh_view_fs_get_view_item(
                view_fs,
                GPOINTER_TO_UINT(iter->data)
            );

        iter->data =
            wintc_sh_view_fs_build_path_for_view_item(
                view_fs,
                item
            );
    }

    return list_items;
}

static WinTCShextViewItem* wintc_sh_view_fs_get_view_item(
    WinTCShViewFS* view_fs,
    guint          item_hash
)
{
    return
        (WinTCShextViewItem*)
        g_hash_table_lookup(
            view_fs->fs_map_entries,
            GUINT_TO_POINTER(item_hash)
        );
}

static void wintc_sh_view_fs_update_new_templates(
    WinTCShViewFS* view_fs
)
{
    GList* templates =
        wintc_sh_fs_get_names_as_list(
            WINTC_ASSETS_DIR G_DIR_SEPARATOR_S "templates",
            FALSE,
            G_FILE_TEST_IS_REGULAR,
            FALSE,
            NULL
        );

    g_clear_list(
        &(view_fs->list_new_templates),
        (GDestroyNotify) clear_new_template
    );

    for (GList* iter = templates; iter; iter = iter->next)
    {
        const gchar* filename = (gchar*) iter->data;

        // Pull out the MIME type parts from filename
        //
        const gchar* p_mime_base_end = strstr(filename, "-");

        if (!p_mime_base_end)
        {
            g_warning(
                "shell: fs - doesn't look like valid MIME: %s",
                filename
            );

            continue;
        }

        // Build the file path...
        //
        gchar* mime_base   = wintc_substr(filename, p_mime_base_end);
        gchar* mime_spec   = wintc_substr(p_mime_base_end + 1, NULL);
        gchar* mime_specfn = g_strdup_printf("%s.xml", mime_spec);

        gchar* mime_path =
            g_build_path(
                G_DIR_SEPARATOR_S,
                G_DIR_SEPARATOR_S,
                "usr",
#ifdef WINTC_PKGMGR_BSDPKG
                "local",
#endif
                "share",
                "mime",
                mime_base,
                mime_specfn,
                NULL
            );

        // Attempt to parse the MIME XML
        //
        xmlDocPtr xml_mime = xmlParseFile(mime_path);

        g_free(mime_base);
        g_free(mime_spec);
        g_free(mime_specfn);
        g_free(mime_path);

        if (!xml_mime)
        {
            WINTC_LOG_DEBUG(
                "shell: fs - could not get XML for %s",
                filename
            );

            continue;
        }

        // Retrieve the name
        //
        xmlNodePtr xml_root = xmlDocGetRootElement(xml_mime);

        for (xmlNodePtr node = xml_root->children; node; node = node->next)
        {
            // We're looking for <comment>
            //
            xmlChar* node_lang;

            if (node->type != XML_ELEMENT_NODE)
            {
                continue;
            }

            if (g_strcmp0((gchar*) node->name, "comment") != 0)
            {
                continue;
            }

            if ((node_lang = xmlNodeGetLang(node)))
            {
                xmlFree(node_lang);
                continue;
            }

            // Create the new template item
            //
            WinTCShViewFSNewTemplate* template =
                g_new(WinTCShViewFSNewTemplate, 1);

            xmlChar* mime_name = xmlNodeGetContent(node);

            template->filename = g_strdup(filename);
            template->name     = g_strdup((gchar*) mime_name);

            xmlFree(mime_name);

            view_fs->list_new_templates =
                g_list_prepend(
                    view_fs->list_new_templates,
                    template
                );

            break;
        }

        xmlFreeDoc(xml_mime);
    }

    if (view_fs->list_new_templates)
    {
        view_fs->list_new_templates =
            g_list_reverse(view_fs->list_new_templates);
    }

    g_list_free_full(
        templates,
        (GDestroyNotify) g_free
    );
}

//
// CALLBACKS
//
static gboolean shopr_delete(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextOperation* operation,
    GtkWindow*           wnd,
    WINTC_UNUSED(GError** error)
)
{
    WinTCShFSOperation* op =
        wintc_sh_fs_operation_new(
            (GList*) operation->priv,
            NULL,
            WINTC_SH_FS_OPERATION_TRASH
        );

    g_signal_connect(
        op,
        "done",
        G_CALLBACK(on_fs_operation_done),
        operation->priv
    );

    wintc_sh_fs_operation_do(op, wnd);

    return TRUE;
}

static gboolean shopr_new(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    WINTC_UNUSED(GtkWindow* wnd),
    GError**             error
)
{
    //
    // FIXME: Localisation needed in the names this func uses
    //

    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    // Folder creation is ID 80, above that and we're dealing with a MIME
    // template
    //
    gint     op        = GPOINTER_TO_INT(operation->priv);
    gboolean is_folder = op == WINTC_SH_VIEW_FS_OP_NEW_FOLDER;

    GFile*                    file;
    guint                     hash;
    gchar*                    path;
    GError*                   local_error = NULL;
    gchar*                    name;
    const gchar*              name_type;
    gboolean                  success;
    WinTCShViewFSNewTemplate* template;

    if (is_folder)
    {
        name_type = "Folder";
    }
    else
    {
        template =
            (WinTCShViewFSNewTemplate*)
            g_list_nth_data(
                view_fs->list_new_templates,
                op - WINTC_SH_VIEW_FS_OP_NEW_FOLDER - 1
            );

        name_type = template->name;
    }

    for (gint attempt = 0; attempt < 100; attempt++)
    {
        if (attempt)
        {
            name =
                g_strdup_printf(
                    "New %s (%d)",
                    name_type,
                    attempt
                );
        }
        else
        {
            name =
                g_strdup_printf(
                    "New %s",
                    name_type
                );
        }

        path = g_build_path(G_DIR_SEPARATOR_S, view_fs->path, name, NULL);
        hash = g_str_hash(path);
        file = g_file_new_for_path(path);

        if (is_folder)
        {
            success =
                g_file_make_directory(
                    file,
                    NULL,
                    &local_error
                );
        }
        else
        {
            gchar* template_path = g_build_path(
                                       G_DIR_SEPARATOR_S,
                                       WINTC_ASSETS_DIR,
                                       "templates",
                                       template->filename,
                                       NULL
                                   );
            GFile* template_file = g_file_new_for_path(template_path);

            success =
                g_file_copy (
                    template_file,
                    file,
                    G_FILE_COPY_NONE,
                    NULL,
                    NULL,
                    NULL,
                    &local_error
                );

            g_free(template_path);
            g_object_unref(template_file);
        }

        g_free(path);
        g_free(name);
        g_object_unref(file);

        if (success)
        {
            view_fs->next_new_hash = hash;
            return TRUE;
        }
        else
        {
            if (local_error->code == G_IO_ERROR_EXISTS)
            {
                g_clear_error(&local_error);
                continue;
            }
            else
            {
                g_propagate_error(error, local_error);
                return FALSE;
            }
        }
    }

    // FIXME: Set error here
    return FALSE;
}

static gboolean shopr_open(
    WinTCIShextView*     view,
    WinTCShextOperation* operation,
    WINTC_UNUSED(GtkWindow* wnd),
    GError**             error
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    GList* targets = operation->priv;

    if (!targets)
    {
        g_warning("%s", "shell: fs open op - no files specified?");
        return TRUE;
    }

    // Iterate over to activate the items
    //
    GError*  local_error = NULL;
    gboolean success     = TRUE;

    for (GList* iter = targets; iter; iter = iter->next)
    {
        WinTCShextViewItem* item =
            g_hash_table_lookup(
                view_fs->fs_map_entries,
                iter->data
            );

        if (!item)
        {
            continue;
        }

        // Valid - attempt launch
        //
        // Only store the first error
        //
        if (
            !real_activate_item(
                view_fs,
                item,
                NULL,
                local_error ? NULL : &local_error
            )
        )
        {
            success = FALSE;
        }
    }

    if (!success && local_error)
    {
        g_propagate_error(error, local_error);
    }

    g_list_free(targets);

    return success;
}

static gboolean shopr_paste(
    WinTCIShextView*     view,
    WINTC_UNUSED(WinTCShextOperation* operation),
    GtkWindow*           wnd,
    GError**             error
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    return wintc_sh_fs_clipboard_paste(
        view_fs->fs_clipboard,
        view_fs->path,
        wnd,
        error
    );
}

static void on_file_monitor_changed(
    WINTC_UNUSED(GFileMonitor* self),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(user_data);

    GList*                    data      = NULL;
    gchar*                    file_path = NULL;
    gboolean                  is_dir;
    WinTCShextViewItem*       item;
    WinTCShextViewItemsUpdate update    = { 0 };

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            file_path = g_file_get_path(file);
            is_dir    =
                g_file_query_file_type(file, 0, NULL) == G_FILE_TYPE_DIRECTORY;

            WINTC_LOG_DEBUG("shell: fs monitor - %s created", file_path);

            item = g_new(WinTCShextViewItem, 1);

            item->display_name = g_file_get_basename(file);
            item->icon_name    = is_dir ?
                                     g_strdup("inode-directory") :
                                     get_file_mime_icon(file);
            item->is_leaf      = !is_dir;
            item->hash         = g_str_hash(file_path);

            // Did we just make this item?
            //
            if (item->hash == view_fs->next_new_hash)
            {
                item->hint = WINTC_SHEXT_VIEW_ITEM_IS_NEW;
                view_fs->next_new_hash = 0;
            }

            g_hash_table_insert(
                view_fs->fs_map_entries,
                GUINT_TO_POINTER(item->hash),
                item
            );

            // Issue update
            //
            data = g_list_prepend(data, item);

            update.data = data;
            update.done = TRUE;

            _wintc_ishext_view_items_added(
                WINTC_ISHEXT_VIEW(view_fs),
                &update
            );

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            file_path = g_file_get_path(file);

            WINTC_LOG_DEBUG("shell: fs monitor - %s deleted", file_path);

            // Issue update
            data =
                g_list_prepend(
                    data,
                    GUINT_TO_POINTER(g_str_hash(file_path))
                );

            update.data = data;
            update.done = TRUE;

            _wintc_ishext_view_items_removed(
                WINTC_ISHEXT_VIEW(view_fs),
                &update
            );

            break;

        default: break;
    }

    g_list_free(data);
    g_free(file_path);
}

static void on_fs_operation_done(
    WinTCShFSOperation* self,
    gpointer            user_data
)
{
    g_list_free_full((GList*) user_data, g_free);
    g_object_unref(self);
}
