#include <gio/gio.h>
#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

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
static void wintc_sh_view_fs_refresh_items(
    WinTCIShextView* view
);
static void wintc_sh_view_fs_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);
static void wintc_sh_view_fs_get_actions_for_view(
    WinTCIShextView* view
);
static guint wintc_sh_view_fs_get_unique_hash(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_fs_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_sh_view_fs_get_icon_name(
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
static gboolean wintc_sh_view_fs_has_parent(
    WinTCIShextView* view
);

static void clear_view_item(
    WinTCShextViewItem* item
);

static void on_file_monitor_changed(
    GFileMonitor*     self,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
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
    gchar*  parent_path;
    gchar*  path;

    GFileMonitor* fs_monitor;
    GHashTable*   fs_map_entries;

    WinTCShextHost* shext_host;
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
    WINTC_UNUSED(WinTCShViewFS* self)
) {}

static void wintc_sh_view_fs_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_sh_view_fs_activate_item;
    iface->refresh_items        = wintc_sh_view_fs_refresh_items;
    iface->get_actions_for_item = wintc_sh_view_fs_get_actions_for_item;
    iface->get_actions_for_view = wintc_sh_view_fs_get_actions_for_view;
    iface->get_display_name     = wintc_sh_view_fs_get_display_name;
    iface->get_icon_name        = wintc_sh_view_fs_get_icon_name;
    iface->get_parent_path      = wintc_sh_view_fs_get_parent_path;
    iface->get_path             = wintc_sh_view_fs_get_path;
    iface->get_unique_hash      = wintc_sh_view_fs_get_unique_hash;
    iface->has_parent           = wintc_sh_view_fs_has_parent;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_fs_dispose(
    GObject* object
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(object);

    g_clear_object(&(view_fs->fs_monitor));
    g_clear_object(&(view_fs->shext_host));

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
            view->path = g_value_dup_string(value);

            // Create parent path string
            //
            if (g_strcmp0(view->path, "/") != 0)
            {
                view->parent_path = g_path_get_dirname(view->path);
            }

            break;

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
    GError*        local_error = NULL;
    WinTCShViewFS* view_fs     = WINTC_SH_VIEW_FS(view);

    WINTC_SAFE_REF_CLEAR(error);

    // Retrieve the item itself
    //
    WinTCShextViewItem* item =
        (WinTCShextViewItem*)
            g_hash_table_lookup(
                view_fs->fs_map_entries,
                GUINT_TO_POINTER(item_hash)
            );

    if (!item)
    {
        g_critical(
            "%s",
            "shell: fs - attempt to activate non-existent item"
        );
        return FALSE;
    }

    // Retrieve MIME for the item
    //
    gchar*  next_path = g_build_path(
                            G_DIR_SEPARATOR_S,
                            view_fs->path,
                            item->display_name,
                            NULL
                        );
    gchar*  mime_type = wintc_query_mime_for_file(
                            next_path,
                            &local_error
                        );

    if (!mime_type)
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    // Handle the item based on MIME, if it's a directory then we can set that
    // as our next location - otherwise, it depends if there is a shell
    // extension for handling the MIME type or not
    //
    if (g_strcmp0(mime_type, "inode/directory") == 0)
    {
        path_info->base_path = g_strdup_printf("file://%s", next_path);
    }
    else
    {
        if (wintc_shext_host_has_view_for_mime(view_fs->shext_host, mime_type))
        {
            path_info->base_path = g_strdup_printf("file://%s", next_path);
        }
        else
        {
            if (!wintc_launch_command(next_path, &local_error))
            {
                g_propagate_error(error, local_error);
            }
        }
    }

    g_free(mime_type);
    g_free(next_path);

    return TRUE;
}

static void wintc_sh_view_fs_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh fs view");

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
        gboolean            is_dir;
        WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

        entry_path =
            g_build_path(
                G_DIR_SEPARATOR_S,
                view_fs->path,
                (gchar*) iter->data,
                NULL
            );

        is_dir = g_file_test(entry_path, G_FILE_TEST_IS_DIR);

        item->display_name = (gchar*) g_steal_pointer(&(iter->data));
        item->icon_name    = is_dir ? "inode-directory" : "empty";
        item->is_leaf      = !is_dir;
        item->hash         = g_str_hash(entry_path);

        g_free(entry_path);

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

static void wintc_sh_view_fs_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

static void wintc_sh_view_fs_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
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
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "inode-directory";
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
// CALLBACKS
//
static void clear_view_item(
    WinTCShextViewItem* item
)
{
    g_free(item->display_name);
    g_free(item);
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
            item->icon_name    = is_dir ? "inode-directory" : "empty";
            item->is_leaf      = !is_dir;
            item->hash         = g_str_hash(file_path);

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
