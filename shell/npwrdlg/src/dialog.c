#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "dialog.h"

//
// PRIVATE ENUMS
//
enum // Class properties
{
    PROP_DIALOG_KIND = 1,
    N_PROPERTIES
};

enum
{
    DIALOG_KIND_POWER_OPTIONS,
    DIALOG_KIND_USER_OPTIONS
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNewPwrDlgDialogPrivate
{
    int blah;
};

struct _WinTCNewPwrDlgDialogClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCNewPwrDlgDialog
{
    GtkApplicationWindow __parent__;

    WinTCNewPwrDlgDialogPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
static void wintc_npwrdlg_dialog_finalize(
    GObject* object
);

static void wintc_npwrdlg_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCNewPwrDlgDialog,
    wintc_npwrdlg_dialog,
    GTK_TYPE_APPLICATION_WINDOW,
    G_ADD_PRIVATE(WinTCNewPwrDlgDialog)
)

static void wintc_npwrdlg_dialog_class_init(
    WinTCNewPwrDlgDialogClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_npwrdlg_dialog_finalize;
    object_class->set_property = wintc_npwrdlg_dialog_set_property;

    g_object_class_install_property (
        object_class,
        PROP_DIALOG_KIND,
        g_param_spec_int (
            "dialog-kind",
            "DialogKind",
            "The kind of power options dialog to display.",
            DIALOG_KIND_POWER_OPTIONS,
            DIALOG_KIND_USER_OPTIONS,
            DIALOG_KIND_POWER_OPTIONS,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_npwrdlg_dialog_init(
    WinTCNewPwrDlgDialog* self
)
{

    gtk_window_set_decorated(GTK_WINDOW(self), FALSE);
    gtk_window_set_title(GTK_WINDOW(self), _("Shutdown Windows"));

    wintc_widget_add_style_class(GTK_WIDGET(self), "npwrdlg");

    //
    // Window population is done via set_property as we need to determine
    // the intended dialog based on dialog-kind property
    //
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_npwrdlg_dialog_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(wintc_npwrdlg_dialog_parent_class)->finalize) (object);
}

static void wintc_npwrdlg_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNewPwrDlgDialog* dlg = WINTC_NPWRDLG_DIALOG(object);

    GtkBuilder* builder;
    GtkWidget*  main_box;

    switch (prop_id)
    {
        case PROP_DIALOG_KIND:
            // Populate the window based on dialog kind
            //
            switch (g_value_get_int(value))
            {
                case DIALOG_KIND_POWER_OPTIONS:
                    builder =
                        gtk_builder_new_from_resource(
                            "/uk/oddmatics/wintc/npwrdlg/pwropts.ui"
                        );

                    break;

                case DIALOG_KIND_USER_OPTIONS:
                    builder =
                        gtk_builder_new_from_resource(
                            "/uk/oddmatics/wintc/npwrdlg/usropts.ui"
                        );

                    break;
            }

            main_box = GTK_WIDGET(gtk_builder_get_object(builder, "main-box"));

            gtk_container_add(GTK_CONTAINER(dlg), main_box);

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_npwrdlg_dialog_new_for_power_options(
    WinTCNewPwrDlgApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_WINTC_NPWRDLG_DIALOG,
            "application", GTK_APPLICATION(app),
            "dialog-kind", DIALOG_KIND_POWER_OPTIONS,
            NULL
        )
    );
}

GtkWidget* wintc_npwrdlg_dialog_new_for_user_options(
    WinTCNewPwrDlgApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_WINTC_NPWRDLG_DIALOG,
            "application", GTK_APPLICATION(app),
            "dialog-kind", DIALOG_KIND_USER_OPTIONS,
            NULL
        )
    );
}
