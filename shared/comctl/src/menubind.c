#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/menubind.h"

//
// FORWARD DECLARATIONS
//
static void wintc_ctl_menu_binding_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCCtlMenuBinding
{
    GObject __parent__;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCCtlMenuBinding,
    wintc_ctl_menu_binding,
    G_TYPE_OBJECT
)

static void wintc_ctl_menu_binding_class_init(
    WinTCCtlMenuBindingClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_ctl_menu_binding_constructed;
}

static void wintc_ctl_menu_binding_init(
    WINTC_UNUSED(WinTCCtlMenuBinding* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_ctl_menu_binding_constructed(
    GObject* object
)
{
    // FIXME: Implement this
}

//
// PUBLIC FUNCTIONS
//
WinTCCtlMenuBinding* wintc_ctl_menu_binding_new(
    GtkMenu*    menu,
    GMenuModel* menu_model
)
{
    return WINTC_CTL_MENU_BINDING(
        g_object_new(
            WINTC_TYPE_CTL_MENU_BINDING,
            "menu",  menu,
            "model", menu_model,
            NULL
        )
    );
}
