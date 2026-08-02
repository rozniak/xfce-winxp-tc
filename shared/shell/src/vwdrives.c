#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../public/nmspace.h"
#include "../public/vwdrives.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SHEXT_HOST = 1,
    PROP_ICON_NAME
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_drives_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_drives_dispose(
    GObject* object
);
static void wintc_sh_view_drives_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_view_drives_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
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
static GList* wintc_sh_view_drives_drag_execute(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_drives_drag_test(
    WinTCIShextView*    view,
    GList*              item_hashes,
    WinTCShextDndTarget target
);
static gboolean wintc_sh_view_drives_drop_execute(
    WinTCIShextView*    view,
    GtkWindow*          wnd,
    guint               item_hash,
    const gchar* const* uris,
    gboolean            hint_copy,
    GError**            error
);
static gboolean wintc_sh_view_drives_drop_test(
    WinTCIShextView*    view,
    guint               item_hash,
    const gchar* const* uris
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
static GMenuModel* wintc_sh_view_drives_get_suggested_actions(
    WinTCIShextView* view,
    guint            item_hash
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

    WinTCShextHost* shext_host;
    GList*          list_tl_items;
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
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_sh_view_drives_dispose;
    object_class->get_property = wintc_sh_view_drives_get_property;
    object_class->set_property = wintc_sh_view_drives_set_property;

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
    iface->drag_execute            = wintc_sh_view_drives_drag_execute;
    iface->drag_test               = wintc_sh_view_drives_drag_test;
    iface->drop_execute            = wintc_sh_view_drives_drop_execute;
    iface->drop_test               = wintc_sh_view_drives_drop_test;
    iface->get_display_name        = wintc_sh_view_drives_get_display_name;
    iface->get_icon_name           = wintc_sh_view_drives_get_icon_name;
    iface->get_items               = wintc_sh_view_drives_get_items;
    iface->get_operations_for_item =
        wintc_sh_view_drives_get_operations_for_item;
    iface->get_operations_for_view =
        wintc_sh_view_drives_get_operations_for_view;
    iface->get_parent_path         = wintc_sh_view_drives_get_parent_path;
    iface->get_path                = wintc_sh_view_drives_get_path;
    iface->get_suggested_actions   =
        wintc_sh_view_drives_get_suggested_actions;
    iface->get_unique_hash         = wintc_sh_view_drives_get_unique_hash;
    iface->has_parent              = wintc_sh_view_drives_has_parent;
    iface->refresh_items           = wintc_sh_view_drives_refresh_items;
    iface->spawn_operation         = wintc_sh_view_drives_spawn_operation;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_drives_dispose(
    GObject* object
)
{
    WinTCShViewDrives* view_drives = WINTC_SH_VIEW_DRIVES(object);

    g_clear_object(&(view_drives->shext_host));
    g_clear_list(&(view_drives->list_tl_items), NULL);

    (G_OBJECT_CLASS(wintc_sh_view_drives_parent_class))
        ->dispose(object);
}

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

static void wintc_sh_view_drives_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShViewDrives* view_drives = WINTC_SH_VIEW_DRIVES(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            view_drives->shext_host = g_value_dup_object(value);
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
    WinTCIShextView*    view,
    guint               item_hash,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShViewDrives* view_drives = WINTC_SH_VIEW_DRIVES(view);

    // FIXME: Crude finding items
    //
    WinTCShextTopLevelItem* found_tl_item = NULL;

    for (GList* iter = view_drives->list_tl_items; iter; iter = iter->next)
    {
        WinTCShextTopLevelItem* tl_item =
            (WinTCShextTopLevelItem*) iter->data;

        if (tl_item->item->hash == item_hash)
        {
            found_tl_item = tl_item;
            break;
        }
    }

    return found_tl_item->activate_cb(
        view_drives->shext_host,
        found_tl_item->item,
        path_info,
        error
    );
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

static GList* wintc_sh_view_drives_drag_execute(
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

static gboolean wintc_sh_view_drives_drag_test(
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

static gboolean wintc_sh_view_drives_drop_execute(
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

static gboolean wintc_sh_view_drives_drop_test(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(guint               item_hash),
    WINTC_UNUSED(const gchar* const* uris)
)
{
    // FIXME: Check - does My Computer accept drops?
    //
    return FALSE;
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
    WinTCIShextView* view
)
{
    WinTCShViewDrives* view_drives = WINTC_SH_VIEW_DRIVES(view);

    // FIXME: Crude, improve later
    //
    GList* items = NULL;

    for (GList* iter = view_drives->list_tl_items; iter; iter = iter->next)
    {
        WinTCShextTopLevelItem* tl_item =
            (WinTCShextTopLevelItem*) iter->data;

        items = g_list_prepend(items, tl_item->item);
    }

    return g_list_reverse(items);
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

static GMenuModel* wintc_sh_view_drives_get_suggested_actions(
    WINTC_UNUSED(WinTCIShextView* view),
    guint item_hash
)
{
    if (item_hash)
    {
        g_critical(
            "%s",
            "shell: vwdrives suggested actions for item not implemented"
        );

        return NULL;
    }

    // Construct the suggested actions menu UI
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource("/uk/oddmatics/wintc/shell/amdrvsvw.ui");

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
    WinTCShViewDrives* view_drives = WINTC_SH_VIEW_DRIVES(view);

    WINTC_LOG_DEBUG("%s", "shell: refresh drives view");

    _wintc_ishext_view_refreshing(view);

    // Update items from toplevel
    //
    if (view_drives->list_tl_items)
    {
        g_list_free(view_drives->list_tl_items);
    }

    view_drives->list_tl_items =
        wintc_shext_host_get_toplevel_items(
            view_drives->shext_host,
            WINTC_SH_GUID_CATEGORY_DRIVES
        );


    // Emit the update
    //
    GList* items = wintc_sh_view_drives_get_items(view);
    WinTCShextViewItemsUpdate update = { 0 };

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
WinTCIShextView* wintc_sh_view_drives_new(
    WinTCShextHost* shext_host
)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DRIVES,
            "shext-host", shext_host,
            NULL
        )
    );
}
