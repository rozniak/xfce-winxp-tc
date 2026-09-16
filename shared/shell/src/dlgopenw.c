#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>

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

enum
{
    COLUMN_ICON_NAME,
    COLUMN_DISPLAY_NAME,
    COLUMN_EXE_PATH,
    N_COLUMNS
};

enum
{
    TREE_ROW_RECOMMENDED,
    TREE_ROW_OTHER
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
    gchar*        file_path;
    GtkTreeStore* tree_model;

    // UI
    //
    GtkWidget* button_browse;
    GtkWidget* button_cancel;
    GtkWidget* button_ok;
    GtkWidget* check_mime;
    GtkWidget* label_file;
    GtkWidget* tree_view;
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

    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        button_browse
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        button_cancel
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        button_ok
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        check_mime
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        label_file
    );
    gtk_widget_class_bind_template_child(
        widget_class,
        WinTCShOpenWithDialog,
        tree_view
    );
}

static void wintc_sh_open_with_dialog_init(
    WinTCShOpenWithDialog* self
)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    // Prepare tree store
    //
    GtkTreeViewColumn* new_column;
    GtkCellRenderer*   new_cell;

    self->tree_model =
        gtk_tree_store_new(
            N_COLUMNS,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_STRING
        );

    gtk_tree_view_set_model(
        GTK_TREE_VIEW(self->tree_view),
        GTK_TREE_MODEL(self->tree_model)
    );

    new_column = gtk_tree_view_column_new();
    new_cell   = gtk_cell_renderer_pixbuf_new();

    gtk_tree_view_column_pack_start(new_column, new_cell, FALSE);
    gtk_tree_view_column_add_attribute(
        new_column,
        new_cell,
        "icon-name",
        COLUMN_ICON_NAME
    );

    new_cell = gtk_cell_renderer_text_new();

    gtk_tree_view_column_pack_end(new_column, new_cell, TRUE);
    gtk_tree_view_column_add_attribute(
        new_column,
        new_cell,
        "text",
        COLUMN_DISPLAY_NAME
    );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(self->tree_view),
        new_column
    );

    // Insert Recommended / Other Programs top level rows
    //
    GtkTreeIter iter;

    gtk_tree_store_append(
        GTK_TREE_STORE(self->tree_model),
        &iter,
        NULL
    );
    gtk_tree_store_set(
        GTK_TREE_STORE(self->tree_model),
        &iter,
        COLUMN_ICON_NAME,    "applications-other",
        COLUMN_DISPLAY_NAME, "Recommended Programs",
        -1
    );

    gtk_tree_store_append(
        GTK_TREE_STORE(self->tree_model),
        &iter,
        NULL
    );
    gtk_tree_store_set(
        GTK_TREE_STORE(self->tree_model),
        &iter,
        COLUMN_ICON_NAME,    "applications-other",
        COLUMN_DISPLAY_NAME, "Other Programs",
        -1
    );
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

    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(object);

    wintc_widget_printf(
        dlg->label_file,
        wintc_basename(dlg->file_path)
    );

    // Init recommended programs
    //
    GtkTreeIter iter_new;
    GtkTreeIter iter_parent;
    GList*      list_programs;
    gchar*      mime_type = wintc_query_mime_for_file(dlg->file_path, NULL);

    list_programs =
        g_app_info_get_recommended_for_type(mime_type);

    gtk_tree_model_iter_nth_child(
        GTK_TREE_MODEL(dlg->tree_model),
        &iter_parent,
        NULL,
        TREE_ROW_RECOMMENDED
    );

    for (GList* iter = list_programs; iter; iter = iter->next)
    {
        GAppInfo* app_info = G_APP_INFO(iter->data);

        gtk_tree_store_append(
             dlg->tree_model,
             &iter_new,
             &iter_parent
        );

        gtk_tree_store_set(
            dlg->tree_model,
            &iter_new,
            COLUMN_ICON_NAME, wintc_icon_get_available_name(g_app_info_get_icon(app_info)),
            COLUMN_DISPLAY_NAME, g_strdup(g_app_info_get_name(app_info)),
            COLUMN_EXE_PATH, g_strdup(g_app_info_get_commandline(app_info)),
            -1
        );
    }

    g_list_free_full(list_programs, (GDestroyNotify) g_object_unref);
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
