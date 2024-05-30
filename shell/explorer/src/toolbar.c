#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "toolbar.h"

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
static void wintc_explorer_toolbar_dispose(
    GObject* object
);
static void wintc_explorer_toolbar_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCExplorerToolbar,
    wintc_explorer_toolbar,
    G_TYPE_OBJECT
)

static void wintc_explorer_toolbar_class_init(
    WinTCExplorerToolbarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_explorer_toolbar_dispose;
    object_class->set_property = wintc_explorer_toolbar_set_property;

    g_object_class_install_property(
        object_class,
        PROP_OWNER_EXPLORER,
        g_param_spec_object(
            "owner-explorer",
            "OwnerExplorer",
            "The Explorer window that owns the toolbar.",
            GTK_TYPE_WIDGET,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_explorer_toolbar_init(
    WinTCExplorerToolbar* self
)
{
    self->toolbar = gtk_toolbar_new();

    g_object_ref_sink(self->toolbar);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_explorer_toolbar_dispose(
    GObject* object
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(object);

    g_clear_object(&(toolbar->owner_explorer_wnd));
    g_clear_object(&(toolbar->toolbar));

    (G_OBJECT_CLASS(wintc_explorer_toolbar_parent_class))->dispose(object);
}

static void wintc_explorer_toolbar_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCExplorerToolbar* toolbar = WINTC_EXPLORER_TOOLBAR(object);

    switch (prop_id)
    {
        case PROP_OWNER_EXPLORER:
            toolbar->owner_explorer_wnd =
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
GtkWidget* wintc_explorer_toolbar_get_toolbar(
    WinTCExplorerToolbar* toolbar
)
{
    return toolbar->toolbar;
}
