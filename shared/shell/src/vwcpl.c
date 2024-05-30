#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/cpl.h"
#include "../public/vwcpl.h"

//
// FORWARD DECLARATIONS
//
static void wintc_sh_view_cpl_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
);

static void wintc_sh_view_cpl_finalize(
    GObject* object
);

static gboolean wintc_sh_view_cpl_activate_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static void wintc_sh_view_cpl_refresh_items(
    WinTCIShextView* view
);

static void wintc_sh_view_cpl_get_actions_for_item(
    WinTCIShextView*    view,
    WinTCShextViewItem* item
);

static void wintc_sh_view_cpl_get_actions_for_view(
    WinTCIShextView* view
);

static const gchar* wintc_sh_view_cpl_get_display_name(
    WinTCIShextView* view
);

static void wintc_sh_view_cpl_get_parent_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);

static void wintc_sh_view_cpl_get_path(
    WinTCIShextView*    view,
    WinTCShextPathInfo* path_info
);

static gboolean wintc_sh_view_cpl_has_parent(
    WinTCIShextView* view
);

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCShViewCplClass
{
    GObjectClass __parent__;
};

struct _WinTCShViewCpl
{
    GObject __parent__;

    // State
    //
    GSList* list_cpls;
    GArray* view_items;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCShViewCpl,
    wintc_sh_view_cpl,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_VIEW,
        wintc_sh_view_cpl_ishext_view_interface_init
    )
)

static void wintc_sh_view_cpl_class_init(
    WinTCShViewCplClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_sh_view_cpl_finalize;
}

static void wintc_sh_view_cpl_init(
    WinTCShViewCpl* self
)
{
    self->view_items = g_array_new(FALSE, TRUE, sizeof (WinTCShextViewItem));
}

static void wintc_sh_view_cpl_ishext_view_interface_init(
    WinTCIShextViewInterface* iface
)
{
    iface->activate_item        = wintc_sh_view_cpl_activate_item;
    iface->refresh_items        = wintc_sh_view_cpl_refresh_items;
    iface->get_actions_for_item = wintc_sh_view_cpl_get_actions_for_item;
    iface->get_actions_for_view = wintc_sh_view_cpl_get_actions_for_view;
    iface->get_display_name     = wintc_sh_view_cpl_get_display_name;
    iface->get_parent_path      = wintc_sh_view_cpl_get_parent_path;
    iface->get_path             = wintc_sh_view_cpl_get_path;
    iface->has_parent           = wintc_sh_view_cpl_has_parent;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_view_cpl_finalize(
    GObject* object
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(object);

    g_clear_slist(
        &(view_cpl->list_cpls),
        (GDestroyNotify) wintc_sh_cpl_applet_free
    );
    g_array_free(view_cpl->view_items, TRUE);

    (G_OBJECT_CLASS(wintc_sh_view_cpl_parent_class))->finalize(object);
}

//
// INTERFACE METHODS
//
static gboolean wintc_sh_view_cpl_activate_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
)
{
    WinTCShCplApplet* applet = (WinTCShCplApplet*) item->priv;

    if (wintc_sh_cpl_applet_is_executable(applet))
    {
        return wintc_launch_command(applet->exec, error);
    }

    path_info->base_path = g_strdup(applet->exec);

    return TRUE;
}

static void wintc_sh_view_cpl_refresh_items(
    WinTCIShextView* view
)
{
    WinTCShViewCpl* view_cpl = WINTC_SH_VIEW_CPL(view);

    // Refresh list
    //
    g_clear_slist(
        &(view_cpl->list_cpls),
        (GDestroyNotify) wintc_sh_cpl_applet_free
    );

    view_cpl->list_cpls = wintc_sh_cpl_applet_get_all();

    g_array_remove_range(
        view_cpl->view_items,
        0,
        view_cpl->view_items->len
    );
    g_array_set_size(
        view_cpl->view_items,
        g_slist_length(view_cpl->list_cpls)
    );

    // Create view items
    //
    gint i = 0;

    for (GSList* iter = view_cpl->list_cpls; iter; iter = iter->next)
    {
        WinTCShCplApplet*   applet    = (WinTCShCplApplet*) iter->data;
        WinTCShextViewItem* view_item = &g_array_index(
                                            view_cpl->view_items,
                                            WinTCShextViewItem,
                                            i
                                        );

        view_item->display_name = applet->display_name;
        view_item->icon_name    = applet->icon_name ?
                                      applet->icon_name :
                                      "image-missing";
        view_item->is_leaf      = wintc_sh_cpl_applet_is_executable(applet);
        view_item->priv         = applet;

        i++;
    }

    // Provide update
    //
    WinTCShextViewItemsAddedData update = { 0 };

    update.items     = &g_array_index(
                           view_cpl->view_items,
                           WinTCShextViewItem,
                           0
                       );
    update.num_items = view_cpl->view_items->len;
    update.done      = TRUE;

    _wintc_ishext_view_items_added(view, &update);
}

static void wintc_sh_view_cpl_get_actions_for_item(
    WINTC_UNUSED(WinTCIShextView* view),
    WINTC_UNUSED(WinTCShextViewItem* item)
)
{
    g_critical("%s Not Implemented", __func__);
}

static void wintc_sh_view_cpl_get_actions_for_view(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    g_critical("%s Not Implemented", __func__);
}

static const gchar* wintc_sh_view_cpl_get_display_name(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    // FIXME: Localisation
    //
    return "Control Panel";
}

static void wintc_sh_view_cpl_get_parent_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_DRIVES)
        );
}

static void wintc_sh_view_cpl_get_path(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextPathInfo* path_info
)
{
    path_info->base_path =
        g_strdup(
            wintc_sh_get_place_path(WINTC_SH_PLACE_CONTROLPANEL)
        );
}

static gboolean wintc_sh_view_cpl_has_parent(
    WINTC_UNUSED(WinTCIShextView* view)
)
{
    return TRUE;
}

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_cpl_new(void)
{
    return WINTC_ISHEXT_VIEW(
        g_object_new(
            WINTC_TYPE_SH_VIEW_CPL,
            NULL
        )
    );
}
