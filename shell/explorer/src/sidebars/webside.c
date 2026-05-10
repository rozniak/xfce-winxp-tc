#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "webside.h"

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_EXPLORER_SIDEBAR_ID_WEB = "web";

//
// FORWARD DECLARATIONS
//
static void wintc_exp_web_sidebar_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpWebSidebarClass
{
    WinTCExplorerSidebarClass __parent__;
};

struct _WinTCExpWebSidebar
{
    WinTCExplorerSidebar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpWebSidebar,
    wintc_exp_web_sidebar,
    WINTC_TYPE_EXPLORER_SIDEBAR
)

static void wintc_exp_web_sidebar_class_init(
    WinTCExpWebSidebarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_web_sidebar_constructed;
}

static void wintc_exp_web_sidebar_init(
    WinTCExpWebSidebar* self
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(self);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/webside.ui"
        );

    wintc_builder_get_objects(
        builder,
        "main-box", &(sidebar->root_widget),
        NULL
    );

    g_object_ref(sidebar->root_widget);

    g_object_unref(builder);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_web_sidebar_constructed(
    WINTC_UNUSED(GObject* object)
) {}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_web_sidebar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_SIDEBAR(
        g_object_new(
            WINTC_TYPE_EXP_WEB_SIDEBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}
