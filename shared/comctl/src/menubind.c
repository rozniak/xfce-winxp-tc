#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/menubind.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_MENU = 1,
    PROP_MENU_MODEL
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCCtlMenuBindingMenu
{
    GtkMenu* menu;
} WinTCCtlMenuBindingMenu;

//
// FORWARD DECLARATIONS
//
static void wintc_ctl_menu_binding_constructed(
    GObject* object
);
static void wintc_ctl_menu_binding_dispose(
    GObject* object
);
static void wintc_ctl_menu_binding_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_ctl_menu_binding_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCCtlMenuBinding
{
    GObject __parent__;

    GtkMenu*    menu;
    GMenuModel* menu_model;
} WinTCCtlMenuBinding;

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

    object_class->constructed  = wintc_ctl_menu_binding_constructed;
    object_class->dispose      = wintc_ctl_menu_binding_dispose;
    object_class->get_property = wintc_ctl_menu_binding_get_property;
    object_class->set_property = wintc_ctl_menu_binding_set_property;

    g_object_class_install_property(
        object_class,
        PROP_MENU,
        g_param_spec_object(
            "menu",
            "Menu",
            "The GTK menu to manage.",
            GTK_TYPE_MENU,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_MENU_MODEL,
        g_param_spec_object(
            "menu-model",
            "MenuModel",
            "The menu model to bind to.",
            G_TYPE_MENU_MODEL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
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
    WinTCCtlMenuBinding* menu_binding = WINTC_CTL_MENU_BINDING(object);

    wintc_container_clear(GTK_CONTAINER(menu_binding->menu));

    // Iterate over the menu model
    //
    gint n_menu_items = g_menu_model_get_n_items(menu_binding->menu_model);

    for (gint i = 0; i < n_menu_items; i++)
    {
        gchar* label = NULL;

        if (
            g_menu_model_get_item_attribute(
                menu_binding->menu_model,
                i,
                G_MENU_ATTRIBUTE_LABEL,
                "s",
                &label
            )
        )
        {
            gtk_menu_shell_append(
                GTK_MENU_SHELL(menu_binding->menu),
                gtk_menu_item_new_with_label(label)
            );

            g_free(label);
        }
    }

    (G_OBJECT_CLASS(wintc_ctl_menu_binding_parent_class))
        ->constructed(object);
}

static void wintc_ctl_menu_binding_dispose(
    GObject* object
)
{
    WinTCCtlMenuBinding* menu_binding = WINTC_CTL_MENU_BINDING(object);

    g_clear_object(&(menu_binding->menu));
    g_clear_object(&(menu_binding->menu_model));

    (G_OBJECT_CLASS(wintc_ctl_menu_binding_parent_class))
        ->dispose(object);
}

static void wintc_ctl_menu_binding_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_ctl_menu_binding_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCCtlMenuBinding* menu_binding = WINTC_CTL_MENU_BINDING(object);

    switch (prop_id)
    {
        case PROP_MENU:
            menu_binding->menu =
                g_value_dup_object(value);

            break;

        case PROP_MENU_MODEL:
            menu_binding->menu_model =
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
WinTCCtlMenuBinding* wintc_ctl_menu_binding_new(
    GtkMenu*    menu,
    GMenuModel* menu_model
)
{
    return WINTC_CTL_MENU_BINDING(
        g_object_new(
            WINTC_TYPE_CTL_MENU_BINDING,
            "menu",       menu,
            "menu-model", menu_model,
            NULL
        )
    );
}
