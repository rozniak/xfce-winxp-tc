#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../sidebar.h"
#include "../window.h"
#include "fldrside.h"

//
// FORWARD DECLARATIONS
//
static void wintc_exp_folders_sidebar_constructed(
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpFoldersSidebarClass
{
    WinTCExplorerSidebarClass __parent__;
};

struct _WinTCExpFoldersSidebar
{
    WinTCExplorerSidebar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpFoldersSidebar,
    wintc_exp_folders_sidebar,
    WINTC_TYPE_EXPLORER_SIDEBAR
)

static void wintc_exp_folders_sidebar_class_init(
    WinTCExpFoldersSidebarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_folders_sidebar_constructed;
}

static void wintc_exp_folders_sidebar_init(
    WinTCExpFoldersSidebar* self
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(self);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/fldrside.ui"
        );

    sidebar->root_widget =
        GTK_WIDGET(
            g_object_ref(
                gtk_builder_get_object(builder, "main-box")
            )
        );

    g_object_unref(builder);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_folders_sidebar_constructed(
    WINTC_UNUSED(GObject* object)
) {}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_folders_sidebar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_SIDEBAR(
        g_object_new(
            WINTC_TYPE_EXP_FOLDERS_SIDEBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}
