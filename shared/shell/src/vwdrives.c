#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/vwdrives.h"

//
// STATIC DATA
//

// FIXME: Temporary - only item is the drive root atm
//
static WinTCShextViewItem s_temp_items[] = {
    { "/",             "drive-harddisk",    FALSE, "file:///" },
    { "Control Panel", "preferences-other", FALSE, NULL       }
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_drives_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

WinTCShextPathInfo* wintc_sh_view_drives_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    GError**            error
);

void wintc_sh_view_drives_refresh_items(
    WinTCIShextView* view
);

void wintc_sh_view_drives_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);

void wintc_sh_view_drives_get_actions_for_view(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_drives_get_display_name(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_drives_get_parent_path(
    WinTCIShextView* view
);

const gchar* wintc_sh_view_drives_get_path(
    WinTCIShextView* view
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
    WINTC_UNUSED(WinTCShViewDrivesClass* klass)
)
{
    // TEMP: Assign CPL view path
    //
    s_temp_items[1].priv = wintc_sh_path_for_guid(WINTC_SH_GUID_CPL);
}

static void wintc_sh_view_drives_init(
    WINTC_UNUSED(WinTCShViewDrives* self)
) {}

static void wintc_sh_view_drives_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_sh_view_drives_activate_item;
    iface->refresh_items        = wintc_sh_view_drives_refresh_items;
    iface->get_actions_for_item = wintc_sh_view_drives_get_actions_for_item;
    iface->get_actions_for_view = wintc_sh_view_drives_get_actions_for_view;
    iface->get_display_name     = wintc_sh_view_drives_get_display_name;
    iface->get_parent_path      = wintc_sh_view_drives_get_parent_path;
    iface->get_path             = wintc_sh_view_drives_get_path;
}

//
// INTERFACE METHODS
//
WinTCShextPathInfo* wintc_sh_view_drives_activate_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WinTCShextViewItem* item,
    WINTC_UNUSED(GError**            error)
)
{
    // TODO: Handle properly, we're using temp items for now
    //
    WinTCShextPathInfo* path_info = g_new0(WinTCShextPathInfo, 1);

    path_info->base_path = g_strdup(item->priv);

    return path_info;
}

void wintc_sh_view_drives_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "shell: refresh drives view");

    // Emit only the root '/' for now
    // TODO: Basically everything in My Computer!
    //
    WinTCShextViewItemsAddedData items = {
        &(s_temp_items[0]),
        G_N_ELEMENTS(s_temp_items),
        TRUE
    };

    _wintc_ishext_view_items_added(view, &items);
}

void wintc_sh_view_drives_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

void wintc_sh_view_drives_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
}

const gchar* wintc_sh_view_drives_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Use shlang!
    //
    return "My Computer";
}

const gchar* wintc_sh_view_drives_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP);
}

const gchar* wintc_sh_view_drives_get_path(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return wintc_sh_get_place_path(WINTC_SH_PLACE_DRIVES);
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_drives_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_DRIVES,
            NULL
        )
    );
}
