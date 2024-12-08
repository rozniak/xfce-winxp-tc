#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/if_uihost.h"
#include "../public/uictl.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_UI_HOST = 1
};

//
// FORWARD DECLARATIONS
//
static void wintc_shext_ui_controller_dispose(
    GObject* object
);
static void wintc_shext_ui_controller_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_shext_ui_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCShextUIControllerPrivate
{
    WinTCIShextUIHost* ui_host;
} WinTCShextUIControllerPrivate;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
    WinTCShextUIController,
    wintc_shext_ui_controller,
    G_TYPE_OBJECT
)

static void wintc_shext_ui_controller_class_init(
    WinTCShextUIControllerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_shext_ui_controller_dispose;
    object_class->get_property = wintc_shext_ui_controller_get_property;
    object_class->set_property = wintc_shext_ui_controller_set_property;

    g_object_class_install_property(
        object_class,
        PROP_UI_HOST,
        g_param_spec_object(
            "ui-host",
            "UIHost",
            "The user interface host that owns the controller.",
            WINTC_TYPE_ISHEXT_UI_HOST,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_shext_ui_controller_init(
    WINTC_UNUSED(WinTCShextUIController* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_shext_ui_controller_dispose(
    GObject* object
)
{
    WinTCShextUIControllerPrivate* priv =
        wintc_shext_ui_controller_get_instance_private(
            WINTC_SHEXT_UI_CONTROLLER(object)
        );

    g_clear_object(&(priv->ui_host));

    (G_OBJECT_CLASS(wintc_shext_ui_controller_parent_class))
        ->dispose(object);
}

static void wintc_shext_ui_controller_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShextUIControllerPrivate* priv =
        wintc_shext_ui_controller_get_instance_private(
            WINTC_SHEXT_UI_CONTROLLER(object)
        );

    switch (prop_id)
    {
        case PROP_UI_HOST:
            g_value_set_object(value, priv->ui_host);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_shext_ui_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShextUIControllerPrivate* priv =
        wintc_shext_ui_controller_get_instance_private(
            WINTC_SHEXT_UI_CONTROLLER(object)
        );

    switch (prop_id)
    {
        case PROP_UI_HOST:
            priv->ui_host = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShextUIController* wintc_shext_ui_controller_new_from_type(
    GType              type,
    WinTCIShextUIHost* ui_host
)
{
    return WINTC_SHEXT_UI_CONTROLLER(
        g_object_new(
            type,
            "ui-host", ui_host,
            NULL
        )
    );
}

WinTCIShextUIHost* wintc_shext_ui_controller_get_ui_host(
    WinTCShextUIController* ui_ctl
)
{
    WinTCShextUIControllerPrivate* priv =
        wintc_shext_ui_controller_get_instance_private(
            WINTC_SHEXT_UI_CONTROLLER(ui_ctl)
        );

    return priv->ui_host;
}
