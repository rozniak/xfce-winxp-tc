#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/wizpage.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_TITLE,
    PROP_SUBTITLE,
    PROP_IS_FINAL_PAGE,
    PROP_IS_EXTERIOR_PAGE,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_wizard97_page_finalize(
    GObject* object
);
static void wintc_wizard97_page_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_wizard97_page_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// STATIC DATA
//
static GParamSpec* wintc_wizard97_page_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCWizard97PagePrivate
{
    gchar*   title;
    gchar*   subtitle;
    gboolean is_final_page;
    gboolean is_exterior_page;
} WinTCWizard97PagePrivate;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_PRIVATE(
    WinTCWizard97Page,
    wintc_wizard97_page,
    GTK_TYPE_BOX
)

static void wintc_wizard97_page_class_init(
    WinTCWizard97PageClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_wizard97_page_finalize;
    object_class->get_property = wintc_wizard97_page_get_property;
    object_class->set_property = wintc_wizard97_page_set_property;

    wintc_wizard97_page_properties[PROP_TITLE] =
        g_param_spec_string(
            "title",
            "Title",
            "The title of the page.",
            NULL,
            G_PARAM_READWRITE
        );
    wintc_wizard97_page_properties[PROP_SUBTITLE] =
        g_param_spec_string(
            "subtitle",
            "Subtitle",
            "The subtitle of the page.",
            NULL,
            G_PARAM_READWRITE
        );
    wintc_wizard97_page_properties[PROP_IS_FINAL_PAGE] =
        g_param_spec_boolean(
            "is-final-page",
            "IsFinalPage",
            "Determines whether this is a final page in the wizard.",
            FALSE,
            G_PARAM_READWRITE
        );
    wintc_wizard97_page_properties[PROP_IS_EXTERIOR_PAGE] =
        g_param_spec_boolean(
            "is-exterior-page",
            "IsExteriorPage",
            "Determines whether this is an exterior page.",
            FALSE,
            G_PARAM_READWRITE
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_wizard97_page_properties
    );
}

static void wintc_wizard97_page_init(
    WINTC_UNUSED(WinTCWizard97Page* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_wizard97_page_finalize(
    GObject* object
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            WINTC_WIZARD97_PAGE(object)
        );

    g_free(priv->title);
    g_free(priv->subtitle);

    (G_OBJECT_CLASS(wintc_wizard97_page_parent_class))
        ->finalize(object);
}

static void wintc_wizard97_page_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            WINTC_WIZARD97_PAGE(object)
        );

    switch (prop_id)
    {
        case PROP_TITLE:
            g_value_set_string(value, priv->title);
            break;

        case PROP_SUBTITLE:
            g_value_set_string(value, priv->subtitle);
            break;

        case PROP_IS_FINAL_PAGE:
            g_value_set_boolean(value, priv->is_final_page);
            break;

        case PROP_IS_EXTERIOR_PAGE:
            g_value_set_boolean(value, priv->is_exterior_page);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_wizard97_page_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            WINTC_WIZARD97_PAGE(object)
        );

    switch (prop_id)
    {
        case PROP_TITLE:
            priv->title = g_value_dup_string(value);
            break;

        case PROP_SUBTITLE:
            priv->subtitle = g_value_dup_string(value);
            break;

        case PROP_IS_FINAL_PAGE:
            priv->is_final_page = g_value_get_boolean(value);
            break;

        case PROP_IS_EXTERIOR_PAGE:
            priv->is_exterior_page = g_value_get_boolean(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
gboolean wintc_wizard97_page_get_is_exterior_page(
    WinTCWizard97Page* wiz_page
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            wiz_page
        );

    return priv->is_exterior_page;
}

gboolean wintc_wizard97_page_get_is_final_page(
    WinTCWizard97Page* wiz_page
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            wiz_page
        );

    return priv->is_final_page;
}

const gchar* wintc_wizard97_page_get_subtitle(
    WinTCWizard97Page* wiz_page
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            wiz_page
        );

    return priv->subtitle;
}

const gchar* wintc_wizard97_page_get_title(
    WinTCWizard97Page* wiz_page
)
{
    WinTCWizard97PagePrivate* priv =
        wintc_wizard97_page_get_instance_private(
            wiz_page
        );

    return priv->title;
}
