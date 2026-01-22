#include <glib.h>
#include <wintc/comgtk.h>

#include "setupclr.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SETUP_WINDOW,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_setup_controller_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_setup_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// STATIC DATA
//
static GParamSpec* wintc_setup_controller_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSetupControllerClass
{
    GObjectClass __parent__;
};

struct _WinTCSetupController
{
    GObject __parent__;

    WinTCSetupWindow* setup_wnd;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCSetupController,
    wintc_setup_controller,
    G_TYPE_OBJECT
)

static void wintc_setup_controller_class_init(
    WinTCSetupControllerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_setup_controller_get_property;
    object_class->set_property = wintc_setup_controller_set_property;

    wintc_setup_controller_properties[PROP_SETUP_WINDOW] =
        g_param_spec_object(
            "setup-window",
            "SetupWindow",
            "The setup window.",
            WINTC_TYPE_SETUP_WINDOW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_setup_controller_properties
    );
}

static void wintc_setup_controller_init(
    WINTC_UNUSED(WinTCSetupController* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_setup_controller_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    //WinTCSetupController* setup_ctl = WINTC_SETUP_CONTROLLER(object);

    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_setup_controller_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCSetupController* setup_ctl = WINTC_SETUP_CONTROLLER(object);

    switch (prop_id)
    {
        case PROP_SETUP_WINDOW:
            setup_ctl->setup_wnd = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCSetupController* wintc_setup_controller_new(
    WinTCSetupWindow* setup_wnd
)
{
    return WINTC_SETUP_CONTROLLER(
        g_object_new(
            WINTC_TYPE_SETUP_CONTROLLER,
            "setup-window", setup_wnd,
            NULL
        )
    );
}

void wintc_setup_controller_begin(
    WINTC_UNUSED(WinTCSetupController* setup)
)
{
    // FIXME: Implement this
}
