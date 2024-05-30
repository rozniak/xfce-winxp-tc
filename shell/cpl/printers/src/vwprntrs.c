#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>

#include "vwprntrs.h"

//
// STATIC DATA
//

// FIXME: Temporary, until we display printers
//
static WinTCShextViewItem s_temp_items[] = {
    { "FPO", "printer", TRUE, NULL }
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static gboolean wintc_cpl_view_printers_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static void wintc_cpl_view_printers_refresh_items(
    WinTCIShextView* view
);

static void wintc_cpl_view_printers_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);

static void wintc_cpl_view_printers_get_actions_for_view(
    WinTCIShextView* view
);

static const gchar* wintc_cpl_view_printers_get_display_name(
    WinTCIShextView* view
);

static void wintc_cpl_view_printers_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);

static void wintc_cpl_view_printers_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);

static gboolean wintc_cpl_view_printers_has_parent(
    WinTCIShextView* view
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCCplViewPrintersClass
{
    GObjectClass __parent__;
};

struct _WinTCCplViewPrinters
{
    GObject __parent__;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCCplViewPrinters,
    wintc_cpl_view_printers,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_cpl_view_printers_ishext_view_interface_init
    )
)

static void wintc_cpl_view_printers_class_init(
    WINTC_UNUSED(WinTCCplViewPrintersClass* klass)
) {}

static void wintc_cpl_view_printers_init(
    WINTC_UNUSED(WinTCCplViewPrinters* self)
) {}

static void wintc_cpl_view_printers_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_cpl_view_printers_activate_item;
    iface->refresh_items        = wintc_cpl_view_printers_refresh_items;
    iface->get_actions_for_item = wintc_cpl_view_printers_get_actions_for_item;
    iface->get_actions_for_view = wintc_cpl_view_printers_get_actions_for_view;
    iface->get_display_name     = wintc_cpl_view_printers_get_display_name;
    iface->get_parent_path      = wintc_cpl_view_printers_get_parent_path;
    iface->get_path             = wintc_cpl_view_printers_get_path;
    iface->has_parent           = wintc_cpl_view_printers_has_parent;
}

//
// INTERFACE METHODS
//
static gboolean wintc_cpl_view_printers_activate_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(WinTCShextViewItem* item),
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    GError** error
)
{
    WINTC_SAFE_REF_CLEAR(error);
    g_critical("%s Not Implemented", __func__);
    return FALSE;
}

static void wintc_cpl_view_printers_refresh_items(
    WinTCIShextView* view
)
{
    WINTC_LOG_DEBUG("%s", "cpl-prntrs: refresh printers view");

    // Emit the FPO printer for now
    // TODO: Implement this!
    //
    WinTCShextViewItemsAddedData items = {
        &(s_temp_items[0]),
        G_N_ELEMENTS(s_temp_items),
        TRUE
    };

    _wintc_ishext_view_items_added(view, &items);
}

static void wintc_cpl_view_printers_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView*    view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

static void wintc_cpl_view_printers_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
}

static const gchar* wintc_cpl_view_printers_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Use shlang
    //
    return "Printers and Faxes";
}

static void wintc_cpl_view_printers_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_CONTROLPANEL)
        );
}

static void wintc_cpl_view_printers_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_PRINTERS)
        );
}

static gboolean wintc_cpl_view_printers_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_cpl_view_printers_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_CPL_VIEW_PRINTERS,
            NULL
        )
    );
}
