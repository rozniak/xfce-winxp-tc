#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/fldropts.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_BROWSE_IN_SAME_WINDOW,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_folder_options_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_folder_options_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// STATIC DATA
//
static GParamSpec* wintc_sh_folder_options_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShFolderOptions
{
    GObject __parent__;

    // Settings
    //
    gboolean browse_in_same_window;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShFolderOptions,
    wintc_sh_folder_options,
    G_TYPE_OBJECT
)

static void wintc_sh_folder_options_class_init(
    WinTCShFolderOptionsClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->get_property = wintc_sh_folder_options_get_property;
    object_class->set_property = wintc_sh_folder_options_set_property;

    wintc_sh_folder_options_properties[PROP_BROWSE_IN_SAME_WINDOW] =
        g_param_spec_boolean(
            "browse-in-same-window",
            "BrowseInSameWindow",
            "Determines whether to open folders in the same window.",
            TRUE,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_sh_folder_options_properties
    );
}

static void wintc_sh_folder_options_init(
    WINTC_UNUSED(WinTCShFolderOptions* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_folder_options_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShFolderOptions* fldr_opts = WINTC_SH_FOLDER_OPTIONS(object);

    switch (prop_id)
    {
        case PROP_BROWSE_IN_SAME_WINDOW:
            g_value_set_boolean(value, fldr_opts->browse_in_same_window);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_sh_folder_options_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShFolderOptions* fldr_opts = WINTC_SH_FOLDER_OPTIONS(object);

    switch (prop_id)
    {
        case PROP_BROWSE_IN_SAME_WINDOW:
            fldr_opts->browse_in_same_window = g_value_get_boolean(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShFolderOptions* wintc_sh_folder_options_new(void)
{
    return WINTC_SH_FOLDER_OPTIONS(
        g_object_new(WINTC_TYPE_SH_FOLDER_OPTIONS, NULL)
    );
}

gboolean wintc_sh_folder_options_get_browse_in_same_window(
    WinTCShFolderOptions* fldr_opts
)
{
    return fldr_opts->browse_in_same_window;
}

void wintc_sh_folder_options_set_browse_in_same_window(
    WinTCShFolderOptions* fldr_opts,
    gboolean              browse_in_same_window
)
{
    fldr_opts->browse_in_same_window = browse_in_same_window;
}
