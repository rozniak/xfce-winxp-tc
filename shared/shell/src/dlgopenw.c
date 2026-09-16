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
    COLUMN_APP_INFO,
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

static void wintc_sh_open_with_dialog_init_programs(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter_parent,
    GList*                 list_programs
);

static void on_button_browse_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_cancel_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_button_ok_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_tree_view_cursor_changed(
    GtkTreeView* self,
    gpointer     user_data
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
            G_TYPE_STRING,
            G_TYPE_POINTER
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

    // Connect signals
    //
    g_signal_connect(
        self->button_browse,
        "clicked",
        G_CALLBACK(on_button_browse_clicked),
        self
    );
    g_signal_connect(
        self->button_cancel,
        "clicked",
        G_CALLBACK(on_button_cancel_clicked),
        self
    );
    g_signal_connect(
        self->button_ok,
        "clicked",
        G_CALLBACK(on_button_ok_clicked),
        self
    );
    g_signal_connect(
        self->tree_view,
        "cursor-changed",
        G_CALLBACK(on_tree_view_cursor_changed),
        self
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

    // Init programs
    //
    GtkTreeIter iter_oth;
    GtkTreeIter iter_rec;
    gchar* mime_type = wintc_query_mime_for_file(dlg->file_path, NULL);

    gtk_tree_model_iter_nth_child(
        GTK_TREE_MODEL(dlg->tree_model),
        &iter_rec,
        NULL,
        TREE_ROW_RECOMMENDED
    );
    gtk_tree_model_iter_nth_child(
        GTK_TREE_MODEL(dlg->tree_model),
        &iter_oth,
        NULL,
        TREE_ROW_OTHER
    );

    wintc_sh_open_with_dialog_init_programs(
        dlg,
        &iter_rec,
        g_app_info_get_recommended_for_type(mime_type)
    );
    wintc_sh_open_with_dialog_init_programs(
        dlg,
        &iter_oth,
        g_app_info_get_fallback_for_type(mime_type)
    );
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

//
// PRIVATE FUNCTIONS
//
static void wintc_sh_open_with_dialog_init_programs(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter_parent,
    GList*                 list_programs
)
{
    GtkTreeIter iter_new;

    for (GList* iter = list_programs; iter; iter = iter->next)
    {
        GAppInfo* app_info = G_APP_INFO(iter->data);

        gchar* icon_name =
            wintc_icon_get_available_name(
                g_app_info_get_icon(app_info)
            );

        gtk_tree_store_append(
             dlg->tree_model,
             &iter_new,
             iter_parent
        );

        gtk_tree_store_set(
            dlg->tree_model,
            &iter_new,
            COLUMN_ICON_NAME,    icon_name,
            COLUMN_DISPLAY_NAME, g_strdup(g_app_info_get_name(app_info)),
            COLUMN_APP_INFO,     g_steal_pointer(&(iter->data)),
            -1
        );
    }

    g_list_free(list_programs);
}

//
// CALLBACKS
//
static void on_button_browse_clicked(
    WINTC_UNUSED(GtkButton* self),
    WINTC_UNUSED(gpointer   user_data)
)
{
    // FIXME: Implement this
}

static void on_button_cancel_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    gtk_window_close(GTK_WINDOW(user_data));
}

static void on_button_ok_clicked(
    WINTC_UNUSED(GtkButton* self),
    gpointer user_data
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(user_data);

    // We try either the app info or command line to launch the file
    //
    GAppInfo*   app_info;
    GError*     error = NULL;
    gchar*      exe_path;
    GtkTreeIter iter;

    wintc_tree_view_get_selected_row(
        GTK_TREE_VIEW(dlg->tree_view),
        &iter
    );

    gtk_tree_model_get(
        GTK_TREE_MODEL(dlg->tree_model),
        &iter,
        COLUMN_EXE_PATH, &exe_path,
        COLUMN_APP_INFO, &app_info,
        -1
    );

    if (app_info)
    {
        GFile* file      = g_file_new_for_path(dlg->file_path);
        GList* list_file = g_list_append(NULL, file);

        if (
            !g_app_info_launch(
                app_info,
                list_file,
                NULL,
                &error
            )
        )
        {
            wintc_display_error_and_clear(&error, NULL);
        }

        g_list_free_full(list_file, (GDestroyNotify) g_object_unref);
    }

    if (exe_path)
    {
        gchar* exec = g_strdup_printf("%s \"%s\"", exe_path, dlg->file_path);

        if (!wintc_launch_command(exec, &error))
        {
            wintc_display_error_and_clear(&error, NULL);
        }

        g_free(exec);
    }

    g_clear_object(&app_info);
    g_free(exe_path);

    gtk_window_close(GTK_WINDOW(dlg));
}

static void on_tree_view_cursor_changed(
    GtkTreeView* self,
    gpointer     user_data
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(user_data);

    GtkTreeIter parent;
    GtkTreeIter selected;
    gboolean    valid =
        wintc_tree_view_get_selected_row(
            self,
            &selected
        ) &&
        gtk_tree_model_iter_parent(
            gtk_tree_view_get_model(self),
            &parent,
            &selected
        );

    gtk_widget_set_sensitive(dlg->button_ok, valid);
}
