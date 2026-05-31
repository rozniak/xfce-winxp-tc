#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "dlggoto.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_RESPONSE,
    PROP_LINE_NUMBER,
    PROP_MAX_LINE_NUMBER,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_notepad_go_to_dialog_constructed(
    GObject* object
);
static void wintc_notepad_go_to_dialog_dispose(
    GObject* object
);
static void wintc_notepad_go_to_dialog_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_notepad_go_to_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean on_button_cancel_clicked(
    GtkButton* self,
    gpointer   user_data
);
static gboolean on_button_ok_clicked(
    GtkButton* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_notepad_go_to_dialog_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotepadGoToDialog
{
    GtkWindow __parent__;

    gint response;

    // State
    //
    gint max_line;

    // UI
    //
    GtkWidget* button_cancel;
    GtkWidget* button_ok;
    GtkWidget* entry_line;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotepadGoToDialog,
    wintc_notepad_go_to_dialog,
    GTK_TYPE_WINDOW
)

static void wintc_notepad_go_to_dialog_class_init(
    WinTCNotepadGoToDialogClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->constructed  = wintc_notepad_go_to_dialog_constructed;
    object_class->dispose      = wintc_notepad_go_to_dialog_dispose;
    object_class->get_property = wintc_notepad_go_to_dialog_get_property;
    object_class->set_property = wintc_notepad_go_to_dialog_set_property;

    wintc_notepad_go_to_dialog_properties[PROP_RESPONSE] =
        g_param_spec_int(
            "response",
            "Response",
            "The response to the dialog",
            G_MININT16,
            0,
            GTK_RESPONSE_CANCEL,
            G_PARAM_READABLE
        );
    wintc_notepad_go_to_dialog_properties[PROP_LINE_NUMBER] =
        g_param_spec_int(
            "line-number",
            "LineNumber",
            "The line number to select.",
            1,
            G_MAXINT16,
            1,
            G_PARAM_READWRITE
        );
    wintc_notepad_go_to_dialog_properties[PROP_MAX_LINE_NUMBER] =
        g_param_spec_int(
            "max-line-number",
            "MaxLineNumber",
            "The maximum line number that can be selected.",
            1,
            G_MAXINT16,
            1,
            G_PARAM_READWRITE
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_notepad_go_to_dialog_properties
    );

    gtk_widget_class_set_template_from_resource(
        widget_class,
        "/uk/oddmatics/wintc/notepad/dlggoto.ui"
    );

    gtk_widget_class_bind_template_child_full(
        widget_class,
        "button-cancel",
        FALSE,
        G_STRUCT_OFFSET(WinTCNotepadGoToDialog, button_cancel)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "button-ok",
        FALSE,
        G_STRUCT_OFFSET(WinTCNotepadGoToDialog, button_ok)
    );
    gtk_widget_class_bind_template_child_full(
        widget_class,
        "entry-line",
        FALSE,
        G_STRUCT_OFFSET(WinTCNotepadGoToDialog, entry_line)
    );
}

static void wintc_notepad_go_to_dialog_init(
    WinTCNotepadGoToDialog* self
)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    self->response = GTK_RESPONSE_CANCEL;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notepad_go_to_dialog_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_notepad_go_to_dialog_parent_class))
        ->constructed(object);

    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(object);

    // Connect signals
    //
    g_signal_connect(
        dlg_goto->button_cancel,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        dlg_goto
    );

    g_signal_connect(
        dlg_goto->button_ok,
        "clicked",
        G_CALLBACK(on_button_ok_clicked),
        dlg_goto
    );
}

static void wintc_notepad_go_to_dialog_dispose(
    GObject* object
)
{
//    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(object);

    (G_OBJECT_CLASS(wintc_notepad_go_to_dialog_parent_class))
        ->dispose(object);
}

static void wintc_notepad_go_to_dialog_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(object);

    switch (prop_id)
    {
        case PROP_RESPONSE:
            g_value_set_int(value, dlg_goto->response);
            break;

        case PROP_LINE_NUMBER:
            g_value_set_int(
                value,
                wintc_notepad_go_to_dialog_get_line_number(dlg_goto)
            );

            break;

        case PROP_MAX_LINE_NUMBER:
            g_value_set_int(value, dlg_goto->max_line);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_notepad_go_to_dialog_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(object);

    switch (prop_id)
    {
        case PROP_LINE_NUMBER:
            wintc_notepad_go_to_dialog_set_line_number(
                dlg_goto,
                g_value_get_int(value)
            );

            break;

        case PROP_MAX_LINE_NUMBER:
            wintc_notepad_go_to_dialog_set_max_line_number(
                dlg_goto,
                g_value_get_int(value)
            );

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notepad_go_to_dialog_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTEPAD_GO_TO_DIALOG,
            NULL
        )
    );
}

gint wintc_notepad_go_to_dialog_get_response(
    WinTCNotepadGoToDialog* dlg_goto
)
{
    return dlg_goto->response;
}

gint wintc_notepad_go_to_dialog_get_line_number(
    WinTCNotepadGoToDialog* dlg_goto
)
{
    const gchar* line_str =
        gtk_entry_get_text(GTK_ENTRY(dlg_goto->entry_line));

    if (!wintc_str_is_ascii_numeric(line_str))
    {
        return -1;
    }

    return strtol(line_str, NULL, 10);
}

gint wintc_notepad_go_to_dialog_get_max_line_number(
    WinTCNotepadGoToDialog* dlg_goto
)
{
    return dlg_goto->max_line;
}

void wintc_notepad_go_to_dialog_set_line_number(
    WinTCNotepadGoToDialog* dlg_goto,
    gint                    line_number
)
{
    gchar* line_str = g_strdup_printf("%d", line_number);

    gtk_entry_set_text(
        GTK_ENTRY(dlg_goto->entry_line),
        line_str
    );

    g_free(line_str);
}

void wintc_notepad_go_to_dialog_set_max_line_number(
    WinTCNotepadGoToDialog* dlg_goto,
    gint                    max_line
)
{
    if (max_line < 1)
    {
        return;
    }

    dlg_goto->max_line = max_line;
}

//
// CALLBACKS
//
static gboolean on_button_cancel_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(user_data);

    dlg_goto->response = GTK_RESPONSE_CANCEL;

    gtk_window_close(GTK_WINDOW(dlg_goto));

    return FALSE;
}

static gboolean on_button_ok_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCNotepadGoToDialog* dlg_goto = WINTC_NOTEPAD_GO_TO_DIALOG(user_data);

    // Check input was valid
    //
    gint line_num = wintc_notepad_go_to_dialog_get_line_number(dlg_goto);

    if (line_num < 1 || line_num > dlg_goto->max_line)
    {
        wintc_messagebox_show(
            GTK_WINDOW(dlg_goto),
            _("Line number out of range."),
            _("Notepad"),
            GTK_BUTTONS_OK,
            GTK_MESSAGE_OTHER
        );

        return FALSE;
    }

    // Acceptable
    //
    dlg_goto->response = GTK_RESPONSE_OK;

    gtk_window_close(GTK_WINDOW(dlg_goto));

    return FALSE;
}
