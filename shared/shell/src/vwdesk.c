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

WinTCShextPathInfo* wintc_sh_view_desktop_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    GError**            error
);

void wintc_sh_view_desktop_refresh_items(
    WinTCIShextView* view
);

void wintc_sh_view_desktop_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);

void wintc_sh_view_desktop_get_actions_for_view(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_desktop_get_display_name(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_desktop_get_parent_path(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_desktop_get_path(
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
}

//
// INTERFACE METHODS
//
WinTCShextPathInfo* wintc_sh_view_desktop_activate_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItem* item,
    GError**            error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    WinTCShextPathInfo* path_info = NULL;
    gchar*              target    = (gchar*) item->priv;

    if (!target)
    {
        // FIXME: Set error
        return NULL;
    }

    path_info = g_new0(WinTCShextPathInfo, 1);

    path_info->base_path = g_strdup(target);

    return path_info;
}

void wintc_sh_view_desktop_refresh_items(
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

void wintc_sh_view_desktop_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

void wintc_sh_view_desktop_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
}

const gchar* wintc_sh_view_desktop_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: LAZY!! Use shlang!
    return "Desktop";
}

const gchar* wintc_sh_view_desktop_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return NULL;
}

const gchar* wintc_sh_view_desktop_get_path(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP);
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
