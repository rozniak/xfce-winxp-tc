#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/wizard97.h>

#include "lnkwiz.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_PATH,
    N_PROPERTIES
};

enum
{
    WIZPAGE_INTRO,
};

//
// FORWARD DECLARATIONS
//
static void wintc_cpl_appwiz_new_link_wizard_constructed(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_dispose(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_finalize(
    GObject* object
);
static void wintc_cpl_appwiz_new_link_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_cpl_appwiz_new_link_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
);

//
// STATIC DATA
//
static GParamSpec*
    wintc_cpl_appwiz_new_link_wizard_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCCplAppwizNewLinkWizardClass
{
    WinTCWizard97WindowClass __parent__;
};

struct _WinTCCplAppwizNewLinkWizard
{
    WinTCWizard97Window __parent__;

    // State
    //
    GFile* file;
    gchar* path;

    // UI stuff
    //
    GtkWidget* button_browse;
    GtkWidget* entry_target;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCCplAppwizNewLinkWizard,
    wintc_cpl_appwiz_new_link_wizard,
    WINTC_TYPE_WIZARD97_WINDOW
)

static void wintc_cpl_appwiz_new_link_wizard_class_init(
    WinTCCplAppwizNewLinkWizardClass* klass
)
{
    GObjectClass*             object_class = G_OBJECT_CLASS(klass);
    WinTCWizard97WindowClass* wizard_class =
        WINTC_WIZARD97_WINDOW_CLASS(klass);

    object_class->constructed  = wintc_cpl_appwiz_new_link_wizard_constructed;
    object_class->dispose      = wintc_cpl_appwiz_new_link_wizard_dispose;
    object_class->finalize     = wintc_cpl_appwiz_new_link_wizard_finalize;
    object_class->set_property = wintc_cpl_appwiz_new_link_wizard_set_property;
    wizard_class->constructing_page =
        wintc_cpl_appwiz_new_link_wizard_constructing_page;

    wintc_cpl_appwiz_new_link_wizard_properties[PROP_PATH] =
        g_param_spec_string(
            "path",
            "Path",
            "The path to the shortcut being created.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_cpl_appwiz_new_link_wizard_properties
    );

    wintc_wizard97_window_class_setup_from_resources(
        wizard_class,
        "/uk/oddmatics/wintc/appwiz/watermk.png",
        NULL,
        "/uk/oddmatics/wintc/appwiz/lnkwizp1.ui",
        NULL
    );
}

static void wintc_cpl_appwiz_new_link_wizard_init(
    WinTCCplAppwizNewLinkWizard* self
)
{
    wintc_wizard97_window_init_wizard(
        WINTC_WIZARD97_WINDOW(self)
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_cpl_appwiz_new_link_wizard_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->constructed(object);

    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    // Touch the file
    //
    lnkwiz->file = g_file_new_for_path(lnkwiz->path);
}

static void wintc_cpl_appwiz_new_link_wizard_dispose(
    GObject* object
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    g_clear_object(&(lnkwiz->file));

    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->dispose(object);
}

static void wintc_cpl_appwiz_new_link_wizard_finalize(
    GObject* object
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    g_free(lnkwiz->path);

    (G_OBJECT_CLASS(wintc_cpl_appwiz_new_link_wizard_parent_class))
        ->finalize(object);
}

static void wintc_cpl_appwiz_new_link_wizard_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(object);

    switch (prop_id)
    {
        case PROP_PATH:
            lnkwiz->path = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_cpl_appwiz_new_link_wizard_constructing_page(
    WinTCWizard97Window* wiz_wnd,
    guint                page_num,
    GtkBuilder*          builder
)
{
    WinTCCplAppwizNewLinkWizard* lnkwiz =
        WINTC_CPL_APPWIZ_NEW_LINK_WIZARD(wiz_wnd);

    switch (page_num)
    {
        case WIZPAGE_INTRO:
            wintc_builder_get_objects(
                builder,
                "button-browse", &(lnkwiz->button_browse),
                "entry-target",  &(lnkwiz->entry_target),
                NULL
            );

            break;

        default: break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_appwiz_new_link_wizard_new(
    const gchar* path
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_CPL_APPWIZ_NEW_LINK_WIZARD,
            "path", path,
            "title", "Create Shortcut",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
