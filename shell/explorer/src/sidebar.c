#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "sidebar.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_OWNER_EXPLORER = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_explorer_sidebar_dispose(
    GObject* object
);
static void wintc_explorer_sidebar_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK TYPE DEFINITIONS & CTOR
//
G_DEFINE_TYPE(
    WinTCExplorerSidebar,
    wintc_explorer_sidebar,
    G_TYPE_OBJECT
)

static void wintc_explorer_sidebar_class_init(
    WinTCExplorerSidebarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_explorer_sidebar_dispose;
    object_class->set_property = wintc_explorer_sidebar_set_property;

    g_object_class_install_property(
        object_class,
        PROP_OWNER_EXPLORER,
        g_param_spec_object(
            "owner-explorer",
            "OwnerExplorer",
            "The Explorer window that owns the sidebar.",
            GTK_TYPE_WIDGET,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_explorer_sidebar_init(
    WINTC_UNUSED(WinTCExplorerSidebar* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_explorer_sidebar_dispose(
    GObject* object
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(object);

    g_clear_object(&(sidebar->owner_explorer_wnd));
    g_clear_object(&(sidebar->root_widget));

    (G_OBJECT_CLASS(wintc_explorer_sidebar_parent_class))->dispose(object);
}

static void wintc_explorer_sidebar_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCExplorerSidebar* sidebar = WINTC_EXPLORER_SIDEBAR(object);

    switch (prop_id)
    {
        case PROP_OWNER_EXPLORER:
            sidebar->owner_explorer_wnd =
                g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_explorer_sidebar_get_root_widget(
    WinTCExplorerSidebar* sidebar
)
{
    return sidebar->root_widget;
}
