#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../sidebar.h"
#include "../window.h"
#include "srchside.h"

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_EXPLORER_SIDEBAR_ID_SEARCH = "search";

//
// FORWARD DECLARATIONS
//
static void wintc_exp_search_sidebar_constructed(
    GObject* object
);

static void on_button_close_clicked(
    GtkWidget* self,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCExpSearchSidebarClass
{
    WinTCExplorerSidebarClass __parent__;
};

struct _WinTCExpSearchSidebar
{
    WinTCExplorerSidebar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCExpSearchSidebar,
    wintc_exp_search_sidebar,
    WINTC_TYPE_EXPLORER_SIDEBAR
)

static void wintc_exp_search_sidebar_class_init(
    WinTCExpSearchSidebarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_exp_search_sidebar_constructed;
}

static void wintc_exp_search_sidebar_init(
    WinTCExpSearchSidebar* self
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(self);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/explorer/srchside.ui"
        );

    GtkWidget* button_close = NULL;

    wintc_builder_get_objects(
        builder,
        "button-close", &button_close,
        "main-box",     &(sidebar->root_widget),
        NULL
    );

    g_object_ref(sidebar->root_widget);

    g_signal_connect(
        button_close,
        "clicked",
        G_CALLBACK(on_button_close_clicked),
        self
    );

    g_object_unref(builder);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_exp_search_sidebar_constructed(
    WINTC_UNUSED(GObject* object)
) {}

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_search_sidebar_new(
    WinTCExplorerWindow* wnd
)
{
    return WINTC_EXPLORER_SIDEBAR(
        g_object_new(
            WINTC_TYPE_EXP_SEARCH_SIDEBAR,
            "owner-explorer", wnd,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void on_button_close_clicked(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    WinTCExplorerSidebar* sidebar =
        WINTC_EXPLORER_SIDEBAR(user_data);

    wintc_explorer_window_toggle_sidebar(
        WINTC_EXPLORER_WINDOW(sidebar->owner_explorer_wnd),
        WINTC_EXPLORER_SIDEBAR_ID_SEARCH
    );
}
