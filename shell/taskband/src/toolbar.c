#include <glib.h>
#include <gtk/gtk.h>

#include "toolbar.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_ROOT_WIDGET = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_taskband_toolbar_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCTaskbandToolbar,
    wintc_taskband_toolbar,
    G_TYPE_OBJECT
)

static void wintc_taskband_toolbar_class_init(
    WinTCTaskbandToolbarClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_taskband_toolbar_get_property;

    g_object_class_install_property(
        object_class,
        PROP_ROOT_WIDGET,
        g_param_spec_object(
            "root-widget",
            "RootWidget",
            "The GtkWidget that is the root of the toolbar.",
            GTK_TYPE_WIDGET,
            G_PARAM_READABLE
        )
    );
}

static void wintc_taskband_toolbar_init(
    WinTCTaskbandToolbar* self
)
{
    self->widget_root = NULL;
}

//
// PRIVATE FUNCTIONS
//
GtkWidget* wintc_taskband_toolbar_get_root_widget(
    WinTCTaskbandToolbar* toolbar
)
{
    return toolbar->widget_root;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_taskband_toolbar_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCTaskbandToolbar* toolbar =
        WINTC_TASKBAND_TOOLBAR(object);

    switch (prop_id)
    {
        case PROP_ROOT_WIDGET:
            g_value_set_object(value, toolbar->widget_root);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}
