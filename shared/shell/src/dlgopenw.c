#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shlang.h>

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

static void wintc_sh_open_with_dialog_get_category_iter(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter,
    gint                   category
);
static gboolean wintc_sh_open_with_dialog_find_iter(
    WinTCShOpenWithDialog* dlg,
    GAppInfo*              app_info,
    const gchar*           exe_path,
    GtkTreeIter*           iter_parent,
    GtkTreeIter*           iter_found
);
static void wintc_sh_open_with_dialog_init_programs(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter_parent,
    GList*                 list_programs
);

static gint cb_compare_app_info_by_name(
    gconstpointer a,
    gconstpointer b
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
    gchar*        mime_type;
    GtkTreeStore* tree_model;

    // UI
    //
    GtkWidget* button_browse;
    GtkWidget* button_cancel;
    GtkWidget* button_ok;
    GtkWidget* check_mime;
    GtkWidget* img_icon;
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
        img_icon
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

    // Register CSS
    //
    GtkCssProvider* css_provider = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_provider,
        "/uk/oddmatics/wintc/shell/dlgopenw.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );
}

static void wintc_sh_open_with_dialog_init(
    WinTCShOpenWithDialog* self
)
{
    gtk_widget_init_template(GTK_WIDGET(self));

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-open-with");

    // Set icon - we must do this here because we can't use GThemedIcon with
    // multiple names in the XML
    //
    static gchar* s_icon_names[] = {
        "open-with",
        "text-x-generic",
        "find"
    };

    GIcon* icon =
        g_themed_icon_new_from_names(
            s_icon_names,
            G_N_ELEMENTS(s_icon_names)
        );

    gtk_image_set_from_gicon(
        GTK_IMAGE(self->img_icon),
        icon,
        GTK_ICON_SIZE_INVALID
    );

    g_object_unref(icon);

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
            G_TYPE_OBJECT
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

    dlg->mime_type = wintc_query_mime_for_file(dlg->file_path, NULL);

    wintc_sh_open_with_dialog_get_category_iter(
        dlg,
        &iter_rec,
        TREE_ROW_RECOMMENDED
    );
    wintc_sh_open_with_dialog_get_category_iter(
        dlg,
        &iter_oth,
        TREE_ROW_OTHER
    );

    wintc_sh_open_with_dialog_init_programs(
        dlg,
        &iter_rec,
        g_app_info_get_recommended_for_type(dlg->mime_type)
    );
    wintc_sh_open_with_dialog_init_programs(
        dlg,
        &iter_oth,
        g_app_info_get_fallback_for_type(dlg->mime_type)
    );

    gtk_tree_view_expand_all(GTK_TREE_VIEW(dlg->tree_view));
}

static void wintc_sh_open_with_dialog_finalize(
    GObject* object
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(object);

    g_free(g_steal_pointer(&(dlg->file_path)));
    g_free(g_steal_pointer(&(dlg->mime_type)));

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
static void wintc_sh_open_with_dialog_get_category_iter(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter,
    gint                   category
)
{
    gtk_tree_model_iter_nth_child(
        GTK_TREE_MODEL(dlg->tree_model),
        iter,
        NULL,
        category
    );
}

static gboolean wintc_sh_open_with_dialog_find_iter(
    WinTCShOpenWithDialog* dlg,
    GAppInfo*              app_info,
    const gchar*           exe_path,
    GtkTreeIter*           iter_parent,
    GtkTreeIter*           iter_found
)
{
    GtkTreeIter   iter;
    GtkTreeModel* model = GTK_TREE_MODEL(dlg->tree_model);

    if (
        !gtk_tree_model_iter_children(
            model,
            &iter,
            iter_parent
        )
    )
    {
        return FALSE;
    }

    do
    {
        GAppInfo* app_info_other;
        gchar*    exe_path_other;
        gint      result;

        if (app_info)
        {
            gchar* disp;

            gtk_tree_model_get(
                model,
                &iter,
                COLUMN_DISPLAY_NAME, &disp,
                COLUMN_APP_INFO, &app_info_other,
                -1
            );

            WINTC_LOG_DEBUG("shell: dlgopenw: checking %s", disp);

            result = 
                g_strcmp0(
                    g_app_info_get_id(app_info),
                    g_app_info_get_id(app_info_other)
                );

            g_object_unref(app_info_other);
        }
        else // exe_path
        {
            gtk_tree_model_get(
                model,
                &iter,
                COLUMN_EXE_PATH, &exe_path_other,
                -1
            );

            result = g_strcmp0(exe_path, exe_path_other);

            g_free(exe_path_other);
        }

        if (result == 0)
        {
            *iter_found = iter;
            return TRUE;
        }
    } while (gtk_tree_model_iter_next(model, &iter));

    return FALSE;
}

static void wintc_sh_open_with_dialog_init_programs(
    WinTCShOpenWithDialog* dlg,
    GtkTreeIter*           iter_parent,
    GList*                 list_programs
)
{
    GtkTreeIter iter_new;

    list_programs =
        g_list_sort(
            list_programs,
            (GCompareFunc) cb_compare_app_info_by_name
        );

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
            COLUMN_APP_INFO,     app_info,
            -1
        );
    }

    g_list_free_full(list_programs, (GDestroyNotify) g_object_unref);
}

//
// CALLBACKS
//
static gint cb_compare_app_info_by_name(
    gconstpointer a,
    gconstpointer b
)
{
    return g_strcmp0(
        g_app_info_get_name(G_APP_INFO(a)),
        g_app_info_get_name(G_APP_INFO(b))
    );
}

static void on_button_browse_clicked(
    WINTC_UNUSED(GtkButton* self),
    WINTC_UNUSED(gpointer   user_data)
)
{
    WinTCShOpenWithDialog* dlg = WINTC_SH_OPEN_WITH_DIALOG(user_data);

    // Set up filter
    //
    GtkFileFilter* filter_programs = gtk_file_filter_new();

    gtk_file_filter_set_name(filter_programs, "Programs");
    gtk_file_filter_add_mime_type(filter_programs, "application/x-executable");

    // Set up file dialog
    //
    GtkWidget* file_dlg =
        gtk_file_chooser_dialog_new(
            "Open With...", // FIXME: Localise
            GTK_WINDOW(dlg),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            wintc_lc_get_control_text(WINTC_CTLTXT_CANCEL, WINTC_PUNC_NONE),
            GTK_RESPONSE_CANCEL,
            wintc_lc_get_control_text(WINTC_CTLTXT_OPEN, WINTC_PUNC_NONE),
            GTK_RESPONSE_ACCEPT,
            NULL
        );

    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(file_dlg),
        "/usr/bin"
    );

    gtk_file_chooser_add_filter(
        GTK_FILE_CHOOSER(file_dlg),
        filter_programs
    );

    // Execute
    //
    gint result = gtk_dialog_run(GTK_DIALOG(file_dlg));

    if (result == GTK_RESPONSE_ACCEPT)
    {
        gchar*      file_path = gtk_file_chooser_get_filename(
                                    GTK_FILE_CHOOSER(file_dlg)
                                );
        GtkTreeIter iter;
        GtkTreeIter iter_rec;
        GtkTreeIter iter_oth;

        wintc_sh_open_with_dialog_get_category_iter(
            dlg,
            &iter_rec,
            TREE_ROW_RECOMMENDED
        );
        wintc_sh_open_with_dialog_get_category_iter(
            dlg,
            &iter_oth,
            TREE_ROW_OTHER
        );

        // Is there a desktop file with this ID?
        //
        gchar*           desktop_id  = g_strdup_printf(
                                           "%s.desktop",
                                           wintc_basename(file_path)
                                       );
        GDesktopAppInfo* desktop_ent = g_desktop_app_info_new(desktop_id);

        if (desktop_ent)
        {
            GAppInfo* app_info = G_APP_INFO(desktop_ent);

            // Do we already know this ID?
            //
            if (
                !wintc_sh_open_with_dialog_find_iter(
                    dlg,
                    app_info,
                    NULL,
                    &iter_rec,
                    &iter
                ) &&
                !wintc_sh_open_with_dialog_find_iter(
                    dlg,
                    app_info,
                    NULL,
                    &iter_oth,
                    &iter
                )
            )
            {
                // Not found - insert it into Other category
                //
                gchar* icon_name = wintc_icon_get_available_name(
                                       g_app_info_get_icon(app_info)
                                   );
                gchar* name      = g_strdup(g_app_info_get_name(app_info));

                gtk_tree_store_append(
                    dlg->tree_model,
                    &iter,
                    &iter_oth
                );

                gtk_tree_store_set(
                    dlg->tree_model,
                    &iter,
                    COLUMN_ICON_NAME,    icon_name,
                    COLUMN_DISPLAY_NAME, name,
                    COLUMN_APP_INFO,     app_info,
                    -1
                );
            }

            g_object_unref(app_info);
        }
        else
        {
            // No app info... do we know the executable yet?
            //
            if (
                !wintc_sh_open_with_dialog_find_iter(
                    dlg,
                    NULL,
                    file_path,
                    &iter_rec,
                    &iter
                ) &&
                !wintc_sh_open_with_dialog_find_iter(
                    dlg,
                    NULL,
                    file_path,
                    &iter_oth,
                    &iter
                )
            )
            {
                gtk_tree_store_append(
                    dlg->tree_model,
                    &iter,
                    &iter_oth
                );

                gtk_tree_store_set(
                    dlg->tree_model,
                    &iter,
                    COLUMN_ICON_NAME,    g_strdup("application-x-executable"),
                    COLUMN_DISPLAY_NAME, g_path_get_basename(file_path),
                    COLUMN_EXE_PATH,     g_strdup(file_path),
                    -1
                );
            }
        }

        g_free(desktop_id);
        g_free(file_path);

        // Select in the tree view
        //
        GtkTreeSelection* selection =
            gtk_tree_view_get_selection(GTK_TREE_VIEW(dlg->tree_view));

        gtk_tree_selection_select_iter(selection, &iter);
    }

    gtk_widget_destroy(file_dlg);
    wintc_focus_window(GTK_WINDOW(dlg));
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
            g_app_info_launch(
                app_info,
                list_file,
                NULL,
                &error
            )
        )
        {
            g_app_info_set_as_last_used_for_type(
                app_info,
                dlg->mime_type,
                NULL
            );

            if (
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(dlg->check_mime))
                )
            {
                g_app_info_set_as_default_for_type(
                    app_info,
                    dlg->mime_type,
                    NULL
                );
            }
        }
        else
        {
            wintc_display_error_and_clear(&error, NULL);
        }

        g_list_free_full(list_file, (GDestroyNotify) g_object_unref);
    }

    if (exe_path)
    {
        gchar* exec = g_strdup_printf("%s \"%s\"", exe_path, dlg->file_path);

        if (wintc_launch_command(exec, &error))
        {
            if (
                gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(dlg->check_mime)
                )
            )
            {
                // We must create a desktop entry for this to work
                //
                app_info =
                    g_app_info_create_from_commandline(
                        exe_path,
                        NULL,
                        G_APP_INFO_CREATE_NONE,
                        NULL
                    );

                g_app_info_set_as_default_for_type(
                    app_info,
                    dlg->mime_type,
                    NULL
                );
            }
        }
        else
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
