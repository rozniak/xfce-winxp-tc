#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/menubind.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_MENU_SHELL = 1,
    PROP_MENU_MODEL,
    PROP_WITH_SEPARATORS
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

    GtkMenuShell* menu_shell;
    GMenuModel*   menu_model;
    gboolean      with_separators;
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
        PROP_MENU_SHELL,
        g_param_spec_object(
            "menu-shell",
            "MenuShell",
            "The GTK menu shell to manage.",
            GTK_TYPE_MENU_SHELL,
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
    g_object_class_install_property(
        object_class,
        PROP_WITH_SEPARATORS,
        g_param_spec_boolean(
            "with-separators",
            "WithSeparators",
            "Whether to spawn separators on the top level.",
            FALSE,
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

    wintc_container_clear(
        GTK_CONTAINER(menu_binding->menu_shell)
    );

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
                menu_binding->menu_shell,
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

    g_clear_object(&(menu_binding->menu_shell));
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
        case PROP_MENU_SHELL:
            menu_binding->menu_shell =
                g_value_dup_object(value);

            break;

        case PROP_MENU_MODEL:
            menu_binding->menu_model =
                g_value_dup_object(value);

            break;

        case PROP_WITH_SEPARATORS:
            menu_binding->with_separators =
                g_value_get_boolean(value);

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
    GtkMenuShell* menu_shell,
    GMenuModel*   menu_model,
    gboolean      with_separators
)
{
    return WINTC_CTL_MENU_BINDING(
        g_object_new(
            WINTC_TYPE_CTL_MENU_BINDING,
            "menu-shell",      menu_shell,
            "menu-model",      menu_model,
            "with-separators", with_separators,
            NULL
        )
    );
}
