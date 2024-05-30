#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>

#include "../public/vwfs.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_PATH = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_fs_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
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
    WinTCShextViewItem* item,
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

static const gchar* wintc_sh_view_fs_get_display_name(
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
    GArray* items;
    gchar*  parent_path;
    gchar*  path;
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

    object_class->finalize     = wintc_sh_view_fs_finalize;
    object_class->get_property = wintc_sh_view_fs_get_property;
    object_class->set_property = wintc_sh_view_fs_set_property;

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
    self->items = g_array_new(FALSE, TRUE, sizeof (WinTCShextViewItem));

    g_array_set_clear_func(
        self->items,
        (GDestroyNotify) clear_view_item
    );
}

static void wintc_sh_view_fs_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_sh_view_fs_activate_item;
    iface->refresh_items        = wintc_sh_view_fs_refresh_items;
    iface->get_actions_for_item = wintc_sh_view_fs_get_actions_for_item;
    iface->get_actions_for_view = wintc_sh_view_fs_get_actions_for_view;
    iface->get_display_name     = wintc_sh_view_fs_get_display_name;
    iface->get_parent_path      = wintc_sh_view_fs_get_parent_path;
    iface->get_path             = wintc_sh_view_fs_get_path;
    iface->has_parent           = wintc_sh_view_fs_has_parent;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_fs_finalize(
    GObject* object
)
{
    WinTCShViewFS* view = WINTC_SH_VIEW_FS(object);

    g_array_free(view->items, TRUE);
    g_free(view->parent_path);
    g_free(view->path);

    (G_OBJECT_CLASS(wintc_sh_view_fs_parent_class))->finalize(object);
}

static void wintc_sh_view_fs_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShViewFS* view = WINTC_SH_VIEW_FS(object);

    switch (prop_id)
    {
        case PROP_PATH:
            g_value_set_string(value, view->path);
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
// INTERFACE METHODS
//
static gboolean wintc_sh_view_fs_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WINTC_SAFE_REF_CLEAR(error);

    gchar* next_path =
        g_build_path(
            G_DIR_SEPARATOR_S,
            view_fs->path,
            item->display_name,
            NULL
        );

    path_info->base_path = g_strdup_printf("file://%s", next_path);

    g_free(next_path);

    return TRUE;
}

static void wintc_sh_view_fs_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewFS* view_fs = WINTC_SH_VIEW_FS(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh fs view");

    // FIXME: Error handling (no way of passing to caller)
    //
    GSList* entries =
        wintc_sh_fs_get_names_as_list(
            view_fs->path,
            FALSE,
            0,
            FALSE,
            NULL
        );

    // Create actual array
    // FIXME: list->array should probs be in comgtk
    //
    gchar* entry_path;
    gint   i = 0;

    g_array_remove_range(
        view_fs->items,
        0,
        view_fs->items->len
    );
    g_array_set_size(
        view_fs->items,
        g_slist_length(entries)
    );

    for (GSList* iter = entries; iter; iter = iter->next)
    {
        gboolean            is_dir;
        WinTCShextViewItem* item =
            &g_array_index(
                view_fs->items,
                WinTCShextViewItem,
                i
            );

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

        g_free(entry_path);

        i++;
    }

    g_slist_free(entries);

    // Provide update
    //
    WinTCShextViewItemsAddedData update = { 0 };

    update.items     = &g_array_index(
                           view_fs->items,
                           WinTCShextViewItem,
                           0
                       );
    update.num_items = view_fs->items->len;
    update.done      = TRUE;

    _wintc_ishext_view_items_added(view, &update);
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
    const gchar* path
)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_FS,
            "path", path,
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
    g_clear_pointer(&(item->display_name), g_free);
}
