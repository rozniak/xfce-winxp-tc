#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/dlgopenw.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_FILE_PATH,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_open_with_dialog_constructed(
    GObject* object
);
static void wintc_sh_open_with_dialog_finalize(
    GObject* object
);
static void wintc_sh_open_with_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// STATIC DATA
//
static GParamSpec* wintc_sh_open_with_dialog_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCShOpenWithDialog
{
    GtkWindow __parent__;

    // State
    //
    gchar* file_path;
} WinTCShOpenWithDialog;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShOpenWithDialog,
    wintc_sh_open_with_dialog,
    GTK_TYPE_WINDOW
)

static void wintc_sh_open_with_dialog_class_init(
    WinTCShOpenWithDialogClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->constructed  = wintc_sh_open_with_dialog_constructed;
    object_class->finalize     = wintc_sh_open_with_dialog_finalize;
    object_class->set_property = wintc_sh_open_with_dialog_set_property;

    wintc_sh_open_with_dialog_properties[PROP_FILE_PATH] =
        g_param_spec_string(
            "file-path",
            "FilePath",
            "The path to the file being opened.",
            NULL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_sh_open_with_dialog_properties
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/uk/oddmatics/wintc/shell/dlgopenw.ui"
    );
}

static void wintc_sh_open_with_dialog_init(
    WinTCShOpenWithDialog* self
)
{
    gtk_widget_init_template(GTK_WIDGET(self));
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_open_with_dialog_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_sh_open_with_dialog_parent_class))
        ->constructed(object);

    //WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(object);
}

static void wintc_sh_open_with_dialog_finalize(
    GObject* object
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(object);

    g_free(g_steal_pointer(&(dlg->file_path)));

    (G_OBJECT_CLASS(wintc_sh_open_with_dialog_parent_class))
        ->finalize(object);
}

static void wintc_sh_open_with_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(object);

    switch (prop_id)
    {
        case PROP_FILE_PATH:
            dlg->file_path = g_value_dup_string(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_sh_open_with_dialog_new(
    const gchar* file_path
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_SH_OPEN_WITH_DIALOG,
            "file-path", file_path,
            NULL
        )
    );
}
