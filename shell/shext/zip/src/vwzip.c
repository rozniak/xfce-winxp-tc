#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <string.h>
#include <zip.h>

#include "vwzip.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_PATH = 1,
    PROP_REL_PATH,
    PROP_ICON_NAME
};

//
// FORWARD DECLARATIONS
//
static void wintc_view_zip_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_view_zip_finalize(
    GObject* object
);
static void wintc_view_zip_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_view_zip_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_view_zip_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gint wintc_view_zip_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
);
static GMenuModel* wintc_view_zip_get_operations_for_item(
    WinTCIShextView* view,
    guint            item_hash
);
static GMenuModel* wintc_view_zip_get_operations_for_view(
    WinTCIShextView* view
);
static const gchar* wintc_view_zip_get_display_name(
    WinTCIShextView* view
);
static const gchar* wintc_view_zip_get_icon_name(
    WinTCIShextView* view
);
static void wintc_view_zip_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static void wintc_view_zip_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);
static guint wintc_view_zip_get_unique_hash(
    WinTCIShextView* view
);
static gboolean wintc_view_zip_has_parent(
    WinTCIShextView* view
);
static void wintc_view_zip_refresh_items(
    WinTCIShextView* view
);

static WinTCShextViewItem* wintc_view_zip_get_view_item(
    WinTCViewZip* view_zip,
    guint         item_hash
);

static guint zip_entry_hash(
    const gchar* zip_file_path,
    const gchar* rel_entry_path
);
static gboolean zip_entry_is_in_dir(
    const gchar* this_dir,
    const gchar* entry,
    gint*        name_offset
);

static void clear_view_item(
    WinTCShextViewItem* item
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCViewZipClass
{
    GObjectClass __parent__;
};

struct _WinTCViewZip
{
    GObject __parent__;

    // ZIP state
    //
    gchar*  parent_path;
    gchar*  rel_path;
    gchar*  zip_uri;

    GHashTable* map_items;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCViewZip,
    wintc_view_zip,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_view_zip_ishext_view_interface_init
    )
)

static void wintc_view_zip_class_init(
    WinTCViewZipClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_view_zip_finalize;
    object_class->get_property = wintc_view_zip_get_property;
    object_class->set_property = wintc_view_zip_set_property;

    g_object_class_override_property(
        object_class,
        PROP_ICON_NAME,
        "icon-name"
    );

    g_object_class_install_property(
        object_class,
        PROP_PATH,
        g_param_spec_string(
            "path",
            "Path",
            "The path of the ZIP file to open.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_REL_PATH,
        g_param_spec_string(
            "relative-path",
            "RelativePath",
            "The relative path within the ZIP file to view.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_view_zip_init(
    WINTC_UNUSED(WinTCViewZip* self)
) {}

static void wintc_view_zip_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item           = wintc_view_zip_activate_item;
    iface->compare_items           = wintc_view_zip_compare_items;
    iface->get_operations_for_item = wintc_view_zip_get_operations_for_item;
    iface->get_operations_for_view = wintc_view_zip_get_operations_for_view;
    iface->get_display_name        = wintc_view_zip_get_display_name;
    iface->get_icon_name           = wintc_view_zip_get_icon_name;
    iface->get_parent_path         = wintc_view_zip_get_parent_path;
    iface->get_path                = wintc_view_zip_get_path;
    iface->get_unique_hash         = wintc_view_zip_get_unique_hash;
    iface->has_parent              = wintc_view_zip_has_parent;
    iface->refresh_items           = wintc_view_zip_refresh_items;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_view_zip_finalize(
    GObject* object
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(object);

    g_free(view_zip->parent_path);
    g_free(view_zip->rel_path);
    g_free(view_zip->zip_uri);

    if (view_zip->map_items)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_zip->map_items))
        );
    }

    (G_OBJECT_CLASS(wintc_view_zip_parent_class))->finalize(object);
}

static void wintc_view_zip_get_property(
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

static void wintc_view_zip_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(object);

    switch (prop_id)
    {
        case PROP_PATH:
            view_zip->zip_uri    = g_value_dup_string(value);
            view_zip->parent_path = g_path_get_dirname(view_zip->zip_uri);
            break;

        case PROP_REL_PATH:
            view_zip->rel_path = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// INTERFACE METHODS
//
static gboolean wintc_view_zip_activate_item(
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    WINTC_SAFE_REF_CLEAR(error);

    WinTCShextViewItem* item =
        wintc_view_zip_get_view_item(view_zip, item_hash);

    if (!item)
    {
        g_critical(
            "%s",
            "shell: zip - attempt to activate non-existent item"
        );
        return FALSE;
    }

    // Handle opening directories
    //
    gboolean is_dir = g_str_has_suffix(item->priv, G_DIR_SEPARATOR_S);

    if (is_dir)
    {
        path_info->base_path     = g_strdup(view_zip->zip_uri);
        path_info->extended_path = g_strdup(item->priv);

        return TRUE;
    }

    // TODO: We'll need to extract files first to tmp to be able to open
    //       them - deal with in a separate issue
    //
    g_set_error(
        error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "ZIP file extraction not implemented yet."
    );

    return FALSE;
}

static gint wintc_view_zip_compare_items(
    WinTCIShextView* view,
    guint            item_hash1,
    guint            item_hash2
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    return wintc_shext_view_item_compare_by_fs_order(
        wintc_view_zip_get_view_item(view_zip, item_hash1),
        wintc_view_zip_get_view_item(view_zip, item_hash2)
    );
}

static GMenuModel* wintc_view_zip_get_operations_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(guint            item_hash)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static GMenuModel* wintc_view_zip_get_operations_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_warning("%s Not Implemented", __func__);
    return NULL;
}

static const gchar* wintc_view_zip_get_display_name(
    WinTCIShextView* view
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    // FIXME: Like FS view in shell, this could be broken maybe if there's
    //        an escaped dir separator in the filename?
    //
    if (view_zip->rel_path)
    {
        // This deals with the fact that the ZIP relative paths are essentially
        // a partial path, dirs are like this:
        //     somedir/
        //     somedir/another/
        //
        // This screws with the usual path APIs which assume absolute paths, or
        // at the very least they don't like trailing slashes - here we get the
        // last component by searching for the second-to-last slash, and return
        // everything after it
        //
        // Of course if the path is only a single component like the first
        // example, then next == end will be true on the very first iteration,
        // in which case we can just return the path as-is
        //
        const gchar* end  = strrchr(view_zip->rel_path, G_DIR_SEPARATOR);
        const gchar* last = view_zip->rel_path;
        const gchar* next = strchr(view_zip->rel_path, G_DIR_SEPARATOR);

        while (next != end)
        {
            next++;

            last = next;
            next = strchr(next, G_DIR_SEPARATOR);
        }

        return last;
    }
    else
    {
        return strrchr(view_zip->zip_uri, G_DIR_SEPARATOR) + 1;
    }
}

static const gchar* wintc_view_zip_get_icon_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return "application-zip";
}

static void wintc_view_zip_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    // If we have a relative path, then use the relative path parent... unless
    // the relative path parent is the root of the zip, in which case just
    // return the zip's path only
    //
    if (view_zip->rel_path)
    {
        // A bit like above, we can't just use g_path_get_dirname because it
        // doesn't like the trailing slash - so here we will deal with
        // retrieving the position of the second-to-last slash, and return
        // everything before it
        //
        // Have a look at the example paths in the above comment and this
        // logic should make sense to you
        //
        const gchar* end  = strrchr(view_zip->rel_path, G_DIR_SEPARATOR);
        const gchar* last = view_zip->rel_path;
        const gchar* next = strchr(view_zip->rel_path, G_DIR_SEPARATOR);

        while (next != end)
        {
            next++;

            last = next;
            next = strchr(next, G_DIR_SEPARATOR);
        }

        path_info->base_path = g_strdup(view_zip->zip_uri);

        if (last != view_zip->rel_path)
        {
            gint   len        = last - view_zip->rel_path;
            gchar* rel_parent = g_malloc0((sizeof(gchar) * len) + 1);

            strncpy(rel_parent, view_zip->rel_path, len);

            path_info->extended_path = rel_parent;
        }

        return;
    }

    path_info->base_path = g_strdup(view_zip->parent_path);
}

static void wintc_view_zip_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    path_info->base_path     = g_strdup(view_zip->zip_uri);
    path_info->extended_path = g_strdup(view_zip->rel_path);
}

static guint wintc_view_zip_get_unique_hash(
    WinTCIShextView* view
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    return
        zip_entry_hash(
            (view_zip->zip_uri + strlen("file://")),
            view_zip->rel_path
        );
}

static gboolean wintc_view_zip_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

static void wintc_view_zip_refresh_items(
    WinTCIShextView* view
)
{
    WinTCViewZip* view_zip = WINTC_VIEW_ZIP(view);

    WINTC_LOG_DEBUG("%s", "shext-zip: refresh zip view");

    _wintc_ishext_view_refreshing(view);

    if (view_zip->map_items)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(view_zip->map_items))
        );
    }

    view_zip->map_items =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) clear_view_item
        );

    // Open the archive
    //
    const gchar* path = view_zip->zip_uri + strlen("file://");

    gint   zip_error = 0;
    zip_t* zip_file  = zip_open(path, 0, &zip_error);

    if (!zip_file)
    {
        zip_error_t zip_error_st;

        zip_error_init_with_code(&zip_error_st, zip_error);

        // FIXME: Need a proper way of returning error to caller!
        g_critical(
            "shext-zip: can't open %s , %s",
            path,
            zip_error_strerror(&zip_error_st)
        );

        zip_error_fini(&zip_error_st);

        return;
    }

    // Read through the entries
    // FIXME: Probably want to cap max num of entries? Check zip bombs to see
    //        what happens - don't want to get nuked by a crazy zip archive
    //
    gint64 n_entries = zip_get_num_entries(zip_file, 0);

    for (gint64 i = 0; i < n_entries; i++)
    {
        const gchar* entry_name = zip_get_name(zip_file, (guint64) i, 0);

        // Only want to add the entries in the current relative dir
        //
        gint name_offset = 0;

        if (!zip_entry_is_in_dir(view_zip->rel_path, entry_name, &name_offset))
        {
            continue;
        }

        gchar*              entry_copy = g_strdup(entry_name);
        WinTCShextViewItem* item       = g_new(WinTCShextViewItem, 1);

        item->display_name = entry_copy + name_offset;
        item->is_leaf      = !g_str_has_suffix(entry_name, G_DIR_SEPARATOR_S);
        item->icon_name    = item->is_leaf ? "empty" : "inode-directory";
        item->hash         = zip_entry_hash(path, entry_name);
        item->priv         = entry_copy;

        g_hash_table_insert(
            view_zip->map_items,
            GUINT_TO_POINTER(item->hash),
            item
        );
    }

    zip_close(zip_file);

    // Emit the entries
    //
    WinTCShextViewItemsUpdate update = { 0 };

    GList* items = g_hash_table_get_values(view_zip->map_items);

    update.data = items;
    update.done = TRUE;

    _wintc_ishext_view_items_added(view, &update);

    g_list_free(items);
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_view_zip_new(
    const gchar* path,
    const gchar* rel_path
)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_VIEW_ZIP,
            "path",          path,
            "relative-path", rel_path,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static WinTCShextViewItem* wintc_view_zip_get_view_item(
    WinTCViewZip* view_zip,
    guint         item_hash
)
{
    return
        (WinTCShextViewItem*)
        g_hash_table_lookup(
            view_zip->map_items,
            GUINT_TO_POINTER(item_hash)
        );
}

static guint zip_entry_hash(
    const gchar* zip_file_path,
    const gchar* rel_entry_path
)
{
    guint hash = g_str_hash(zip_file_path);

    if (rel_entry_path)
    {
        hash = hash * 33 + g_str_hash(rel_entry_path);
    }

    return hash;
}

static gboolean zip_entry_is_in_dir(
    const gchar* this_dir,
    const gchar* entry,
    gint*        name_offset
)
{
    const gchar* next_ds;

    *name_offset = 0;

    // If we're at the root of the zip, then we just scrub off any file that is
    // in a subdir, nothing else needed
    //
    if (!this_dir)
    {
        next_ds = strchr(entry, G_DIR_SEPARATOR);

        return !next_ds || next_ds == (entry + strlen(entry) - 1);
    }

    // Exclude the dir itself
    //
    if (g_strcmp0(entry, this_dir) == 0)
    {
        return FALSE;
    }

    // We're in a subdir, so any file not in this subdir is obviously binned
    //
    if (!g_str_has_prefix(entry, this_dir))
    {
        return FALSE;
    }

    // Finally, check that there are no more directory components after our
    // path
    //
    const gchar* entry_in_dir = entry + strlen(this_dir);

    next_ds = strchr(entry_in_dir, G_DIR_SEPARATOR);

    if (!next_ds || next_ds == (entry + strlen(entry) - 1))
    {
        *name_offset = entry_in_dir - entry;
        return TRUE;
    }

    return FALSE;
}

//
// CALLBACKS
//
static void clear_view_item(
    WinTCShextViewItem* item
)
{
    // Don't need to bin display_name as it's an offset into priv which
    // contains the full path in the zip
    //
    g_free(item->priv);
    g_free(item);
}
