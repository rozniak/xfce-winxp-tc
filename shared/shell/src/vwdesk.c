#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/vwdesk.h"

//
// STATIC DATA
//

// FIXME: LAZY AGAIN! Use shlang!!!!!! Temporary as well cos the user can
//        toggle which items are present
//
static WinTCShextViewItem s_desktop_items[] = {
    {
        "My Computer",
        "computer",
        FALSE,
        NULL
    },
    {
        "My Documents",
        "folder-documents",
        FALSE,
        NULL
    },
    {
        "My Network Places",
        "network-workgroup",
        FALSE,
        NULL
    },
    {
        "Recycle Bin",
        "user-trash",
        FALSE,
        NULL
    }
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_desktop_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static gboolean wintc_sh_view_desktop_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static void wintc_sh_view_desktop_refresh_items(
    WinTCIShextView* view
);

static void wintc_sh_view_desktop_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);

static void wintc_sh_view_desktop_get_actions_for_view(
    WinTCIShextView* view
);

static const gchar* wintc_sh_view_desktop_get_display_name(
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

static gboolean wintc_sh_view_desktop_has_parent(
    WinTCIShextView* view
);

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
    WINTC_UNUSED(WinTCShViewDesktopClass* klass)
)
{
    // Assign GUID paths to built-in desktop items - kind of rubbish but
    // whatever
    //
    s_desktop_items[0].priv = wintc_sh_path_for_guid(WINTC_SH_GUID_DRIVES);
}

static void wintc_sh_view_desktop_init(
    WINTC_UNUSED(WinTCShViewDesktop* self)
) {}

static void wintc_sh_view_desktop_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_sh_view_desktop_activate_item;
    iface->refresh_items        = wintc_sh_view_desktop_refresh_items;
    iface->get_actions_for_item = wintc_sh_view_desktop_get_actions_for_item;
    iface->get_actions_for_view = wintc_sh_view_desktop_get_actions_for_view;
    iface->get_display_name     = wintc_sh_view_desktop_get_display_name;
    iface->get_parent_path      = wintc_sh_view_desktop_get_parent_path;
    iface->get_path             = wintc_sh_view_desktop_get_path;
    iface->has_parent           = wintc_sh_view_desktop_has_parent;
}

//
// INTERFACE METHODS
//
static gboolean wintc_sh_view_desktop_activate_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    gchar* target = (gchar*) item->priv;

    if (!target)
    {
        g_critical("%s", "shell: desk view can't activate item, no target");
        return FALSE;
    }

    path_info->base_path = g_strdup(target);

    return TRUE;
}

static void wintc_sh_view_desktop_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "shell: refresh desktop view");

    // Just emit the default items for now
    // TODO: Should aggregate with user desktop files
    //
    WinTCShextViewItemsAddedData items = {
        &(s_desktop_items[0]),
        G_N_ELEMENTS(s_desktop_items),
        TRUE
    };

    _wintc_ishext_view_items_added(view, &items);
}

static void wintc_sh_view_desktop_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

static void wintc_sh_view_desktop_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
}

static const gchar* wintc_sh_view_desktop_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: LAZY!! Use shlang!
    return "Desktop";
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

static gboolean wintc_sh_view_desktop_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return FALSE;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_desktop_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DESKTOP,
            NULL
        )
    );
}
