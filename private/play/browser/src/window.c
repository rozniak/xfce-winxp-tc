#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// STATIC DATA
//
static const gchar* s_program_title = "File Browser";

//
// PRIVATE ENUMS
//
enum
{
    COLUMN_FILENAME = 0,
    N_COLUMNS
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCBrowserWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCBrowserWindow
{
    GtkApplicationWindow __parent__;

    // State
    //
    gchar* curdir;

    gboolean opt_show_hidden;

    // View data
    //
    GtkListStore* list_store;

    // UI
    //
    GtkTreeViewColumn* column_filename;

    GtkWidget* scrollview;
    GtkWidget* treeview;
};

//
// FORWARD DECLARATIONS
//
static void wintc_browser_window_dispose(
    GObject* gobject
);
static void wintc_browser_window_finalize(
    GObject* gobject
);

static void on_treeview_row_activated(
    GtkTreeView*       self,
    GtkTreePath*       path,
    GtkTreeViewColumn* column,
    gpointer           user_data
);

static void navigate_to_dir(
    WinTCBrowserWindow* wnd,
    const gchar*        dirname
);
static void refresh_model(
    WinTCBrowserWindow* wnd,
    GDir*               dir
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCBrowserWindow,
    wintc_browser_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_browser_window_class_init(
    WinTCBrowserWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose  = wintc_browser_window_dispose;
    object_class->finalize = wintc_browser_window_finalize;
}

static void wintc_browser_window_init(
    WinTCBrowserWindow* self
)
{
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    // Options
    //
    self->opt_show_hidden = FALSE;

    // Data storage
    //
    self->list_store = gtk_list_store_new(1, G_TYPE_STRING);

    // Set up UI
    //
    GtkCellRenderer* cell_renderer = gtk_cell_renderer_text_new();

    self->scrollview      = gtk_scrolled_window_new(NULL, NULL);
    self->treeview        = gtk_tree_view_new();
    self->column_filename = gtk_tree_view_column_new();

    gtk_tree_view_column_set_title(self->column_filename, "Filename");

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(self->treeview),
        self->column_filename
    );

    gtk_tree_view_column_pack_start(
        self->column_filename,
        cell_renderer,
        TRUE
    );
    gtk_tree_view_column_add_attribute(
        self->column_filename,
        cell_renderer,
        "text",
        COLUMN_FILENAME
    );

    g_signal_connect(
        self->treeview,
        "row-activated",
        G_CALLBACK(on_treeview_row_activated),
        self
    );

    gtk_container_add(GTK_CONTAINER(self->scrollview), self->treeview);
    gtk_container_add(GTK_CONTAINER(self),             self->scrollview);

    // Navigate to home dir first
    //
    navigate_to_dir(self, g_get_home_dir());
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_browser_window_dispose(
    GObject* gobject
)
{
    WinTCBrowserWindow* wnd = WINTC_BROWSER_WINDOW(gobject);

    g_clear_object(&(wnd->list_store));

    (G_OBJECT_CLASS(wintc_browser_window_parent_class))->dispose(gobject);
}

static void wintc_browser_window_finalize(
    GObject* gobject
)
{
    WinTCBrowserWindow* wnd = WINTC_BROWSER_WINDOW(gobject);

    g_free(wnd->curdir);

    (G_OBJECT_CLASS(wintc_browser_window_parent_class))->finalize(gobject);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_browser_window_new(
    WinTCBrowserApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_BROWSER_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       s_program_title,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void navigate_to_dir(
    WinTCBrowserWindow* wnd,
    const gchar*        dirname
)
{
    GError* error    = NULL;
    GDir*   next_dir = g_dir_open(dirname, 0, &error);

    if (next_dir == NULL)
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    if (wnd->curdir != NULL)
    {
        g_free(wnd->curdir);
    }

    wnd->curdir = g_strdup(dirname);
    refresh_model(wnd, next_dir);

    g_dir_close(next_dir);

    // Update window title
    //
    gchar* new_title =
        g_strdup_printf("%s - %s", s_program_title, wnd->curdir);

    gtk_window_set_title(GTK_WINDOW(wnd), new_title);

    g_free(new_title);
}

static void refresh_model(
    WinTCBrowserWindow* wnd,
    GDir*         dir
)
{
    // Disconnect model
    //
    gtk_tree_view_set_model(GTK_TREE_VIEW(wnd->treeview), NULL);

    // Populate now
    //
    GtkTreeIter  iter;
    const gchar* next_name = NULL;

    gtk_list_store_clear(wnd->list_store);

    gtk_list_store_append(wnd->list_store, &iter);
    gtk_list_store_set(
        wnd->list_store,
        &iter,
        COLUMN_FILENAME, "..",
        -1
    );

    while ((next_name = g_dir_read_name(dir)) != NULL)
    {
        if (!wnd->opt_show_hidden && g_str_has_prefix(next_name, "."))
        {
            continue;
        }

        gtk_list_store_append(wnd->list_store, &iter);
        gtk_list_store_set(
            wnd->list_store,
            &iter,
            COLUMN_FILENAME, next_name,
            -1
        );
    }

    // Reconnect model
    //
    gtk_tree_view_set_model(
        GTK_TREE_VIEW(wnd->treeview),
        GTK_TREE_MODEL(wnd->list_store)
    );
}

//
// CALLBACKS
//
static void on_treeview_row_activated(
    GtkTreeView*       self,
    GtkTreePath*       path,
    WINTC_UNUSED(GtkTreeViewColumn* column),
    gpointer           user_data
)
{
    GtkTreeIter         iter;
    GtkTreeModel*       model = gtk_tree_view_get_model(self);
    WinTCBrowserWindow* wnd   = WINTC_BROWSER_WINDOW(user_data);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gchar* node_name;

        gtk_tree_model_get(
            model,
            &iter,
            COLUMN_FILENAME, &node_name,
            -1
        );

        if (g_strcmp0(node_name, "..") == 0)
        {
            if (g_strcmp0(wnd->curdir, G_DIR_SEPARATOR_S) != 0)
            {
                gchar* last_sep = g_strrstr(wnd->curdir, G_DIR_SEPARATOR_S);

                if (last_sep == wnd->curdir) // Traversing up to root
                {
                    navigate_to_dir(wnd, "/");
                }
                else
                {
                    gchar* parent_path =
                        g_strndup(
                            wnd->curdir,
                            last_sep - wnd->curdir
                        );

                    navigate_to_dir(wnd, parent_path);

                    g_free(parent_path);
                }
            }
        }
        else
        {
            gchar* new_path =
                g_build_path(
                    G_DIR_SEPARATOR_S,
                    wnd->curdir,
                    node_name,
                    NULL
                );

            navigate_to_dir(wnd, new_path);

            g_free(new_path);
        }

        g_free(node_name);
    }
}
