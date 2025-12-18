#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/browser.h"
#include "../public/trevwbeh.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_BROWSER = 1,
    PROP_TREE_VIEW
};

enum
{
    COLUMN_ICON_NAME = 0,
    COLUMN_ENTRY_NAME,
    COLUMN_VIEW_HASH,
    COLUMN_MAPPED_VIEW,
    N_COLUMNS
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_tree_view_behaviour_constructed(
    GObject* object
);
static void wintc_sh_tree_view_behaviour_dispose(
    GObject* object
);
static void wintc_sh_tree_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_sh_tree_view_behaviour_node_add_items(
    WinTCShTreeViewBehaviour* behaviour,
    GtkTreeIter*              node,
    GList*                    items
);
static void wintc_sh_tree_view_behaviour_update_view(
    WinTCShTreeViewBehaviour* behaviour,
    WinTCShBrowser*           browser
);

static void clear_tree_row_reference(
    GtkTreeRowReference* row_ref
);

static void on_browser_load_changed(
    WinTCShBrowser*         self,
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
);

static void on_view_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);
static void on_view_items_removed(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShTreeViewBehaviourClass
{
    GObjectClass __parent__;
};

struct _WinTCShTreeViewBehaviour
{
    GObject __parent__;

    WinTCShBrowser* browser;
    WinTCShextHost* shext_host;

    GtkTreeStore* tree_model;
    GHashTable*   map_hash_to_row;

    GtkWidget* tree_view;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShTreeViewBehaviour,
    wintc_sh_tree_view_behaviour,
    G_TYPE_OBJECT
)

static void wintc_sh_tree_view_behaviour_class_init(
    WinTCShTreeViewBehaviourClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_tree_view_behaviour_constructed;
    object_class->dispose      = wintc_sh_tree_view_behaviour_dispose;
    object_class->set_property = wintc_sh_tree_view_behaviour_set_property;

    g_object_class_install_property(
        object_class,
        PROP_BROWSER,
        g_param_spec_object(
            "browser",
            "Browser",
            "The shell browser instance to bind to.",
            WINTC_TYPE_SH_BROWSER,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_TREE_VIEW,
        g_param_spec_object(
            "tree-view",
            "TreeView",
            "The tree view to manage.",
            GTK_TYPE_TREE_VIEW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_sh_tree_view_behaviour_init(
    WinTCShTreeViewBehaviour* self
)
{
    // Set up map for hash->row
    //
    // The intention of this is to be able to map view/viewitems to a node in
    // the tree, and then those nodes to a concrete view object
    //
    // This is such that views can be essentially lazy-loaded - view items
    // that are enumerated from a view are akin to placeholders, and a view is
    // created only when those nodes are expanded/enumerated themselves
    //
    self->map_hash_to_row =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) clear_tree_row_reference
        );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_tree_view_behaviour_constructed(
    GObject* object
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(object);

    if (!behaviour->browser || !behaviour->tree_view)
    {
        g_critical("%s", "ShTreeViewBehaviour: Must have a browser and view!");
        return;
    }

    behaviour->shext_host =
        g_object_ref(wintc_sh_browser_get_shext_host(behaviour->browser));

    // Clear out tree view in case for some reason there's already stuff in it
    //
    GList* columns = gtk_tree_view_get_columns(
                         GTK_TREE_VIEW(behaviour->tree_view)
                     );

    for (GList* iter = columns; iter; iter = iter->next)
    {
        gtk_tree_view_remove_column(
            GTK_TREE_VIEW(behaviour->tree_view),
            GTK_TREE_VIEW_COLUMN(iter->data)
        );
    }

    g_list_free(columns);

    // Set up tree view
    //
    GtkTreeViewColumn* new_column;
    GtkCellRenderer*   new_cell;

    behaviour->tree_model =
        gtk_tree_store_new(
            4,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_UINT,
            G_TYPE_OBJECT
        );

    gtk_tree_view_set_headers_visible(
        GTK_TREE_VIEW(behaviour->tree_view),
        FALSE
    );
    gtk_tree_view_set_model(
        GTK_TREE_VIEW(behaviour->tree_view),
        GTK_TREE_MODEL(behaviour->tree_model)
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
        COLUMN_ENTRY_NAME
    );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(behaviour->tree_view),
        new_column
    );

    // Hook up everything for getting this all started!
    //
    g_signal_connect_object(
        behaviour->browser,
        "load-changed",
        G_CALLBACK(on_browser_load_changed),
        behaviour,
        G_CONNECT_DEFAULT
    );

    wintc_sh_tree_view_behaviour_update_view(behaviour, behaviour->browser);

    (G_OBJECT_CLASS(wintc_sh_tree_view_behaviour_parent_class))
        ->constructed(object);
}

static void wintc_sh_tree_view_behaviour_dispose(
    GObject* object
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(object);

    g_clear_object(&(behaviour->browser));
    g_clear_object(&(behaviour->shext_host));
    g_clear_object(&(behaviour->tree_view));

    g_hash_table_unref(g_steal_pointer(&(behaviour->map_hash_to_row)));

    (G_OBJECT_CLASS(wintc_sh_tree_view_behaviour_parent_class))
        ->dispose(object);
}

static void wintc_sh_tree_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(object);

    switch (prop_id)
    {
        case PROP_BROWSER:
            behaviour->browser = g_value_dup_object(value);
            break;

        case PROP_TREE_VIEW:
            behaviour->tree_view = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShTreeViewBehaviour* wintc_sh_tree_view_behaviour_new(
    GtkTreeView*    tree_view,
    WinTCShBrowser* browser
)
{
    return WINTC_SH_TREE_VIEW_BEHAVIOUR(
        g_object_new(
            WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR,
            "browser",   browser,
            "tree-view", tree_view,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_sh_tree_view_behaviour_node_add_items(
    WinTCShTreeViewBehaviour* behaviour,
    GtkTreeIter*              node,
    GList*                    items
)
{
    GtkTreeIter         child;
    WinTCShextViewItem* view_item;

    WinTCIShextView* view = NULL;

    gtk_tree_model_get(
        GTK_TREE_MODEL(behaviour->tree_model),
        node,
        COLUMN_MAPPED_VIEW, &view,
        -1
    );

    for (GList* iter = items; iter; iter = iter->next)
    {
        view_item = iter->data;

        // Skip leaf nodes and nodes that already exist
        //
        if (
            view_item->is_leaf ||
            g_hash_table_contains(
                behaviour->map_hash_to_row,
                GUINT_TO_POINTER(view_item->hash)
            )
        )
        {
            continue;
        }

        WINTC_LOG_DEBUG(
            "shell: tree adding item %p",
            GUINT_TO_POINTER(view_item->hash)
        );

        // Sort into model
        //
        GCompareFunc sort_func = wintc_ishext_view_get_sort_func(view);

        gint item_pos =
            wintc_tree_model_get_insertion_sort_pos(
                GTK_TREE_MODEL(behaviour->tree_model),
                node,
                COLUMN_VIEW_HASH,
                G_TYPE_UINT,
                sort_func,
                GUINT_TO_POINTER(view_item->hash)
            );

        // Push to model
        //
        gtk_tree_store_insert(
            behaviour->tree_model,
            &child,
            node,
            item_pos
        );
        gtk_tree_store_set(
            behaviour->tree_model,
            &child,
            COLUMN_ICON_NAME,  view_item->icon_name,
            COLUMN_ENTRY_NAME, view_item->display_name,
            COLUMN_VIEW_HASH,  view_item->hash,
            -1
        );

        // Map hash-->row
        //
        GtkTreePath* tree_path =
            gtk_tree_model_get_path(
                GTK_TREE_MODEL(behaviour->tree_model),
                &child
            );

        g_hash_table_insert(
            behaviour->map_hash_to_row,
            GUINT_TO_POINTER(view_item->hash),
            gtk_tree_row_reference_new(
                GTK_TREE_MODEL(behaviour->tree_model),
                tree_path
            )
        );

        gtk_tree_path_free(tree_path);
    }
}

static void wintc_sh_tree_view_behaviour_update_view(
    WinTCShTreeViewBehaviour* behaviour,
    WinTCShBrowser*           browser
)
{
    WinTCIShextView* current_view =
        wintc_sh_browser_get_current_view(browser);

    if (!current_view)
    {
        return;
    }

    // Collect up the views back up the tree, until we hit a view we already
    // have
    //
    GError*              error      = NULL;
    GSList*              list_views = NULL;
    WinTCShextPathInfo   path_info  = { 0 };
    GtkTreeRowReference* row_ref = NULL;
    WinTCIShextView*     view       = current_view;

    while (view)
    {
        // Collect this view
        //
        list_views = g_slist_append(list_views, view);

        // If there is a node in the tree for the view hash, stop here
        //
        row_ref =
            g_hash_table_lookup(
                behaviour->map_hash_to_row,
                GUINT_TO_POINTER(wintc_ishext_view_get_unique_hash(view))
            );

        if (row_ref)
        {
            break;
        }

        // See if there's a parent to step up to
        //
        if (wintc_ishext_view_has_parent(view))
        {
            wintc_ishext_view_get_parent_path(view, &path_info);

            view =
                wintc_shext_host_get_view_for_path(
                    behaviour->shext_host,
                    &path_info,
                    &error
                );

            wintc_shext_path_info_free_data(&path_info);

            if (!view)
            {
                wintc_log_error_and_clear(&error);
            }
        }
        else
        {
            view = NULL;
        }
    }

    // Reverse list (bottom up) and append items in the tree
    //
    // NOTE ON REFERENCES:
    // Since the list contains the view owned by the browser as well as views
    // instantiated by this function, g_object_ref() needs to only be called on
    // the former (so only one ref is owned by the hash table and will get
    // binned during dispose())
    //
    guint            hash;
    gboolean         new_view;
    GtkTreeIter      next;
    WinTCIShextView* next_view;
    GtkTreeIter      last = { 0 };
    gboolean         should_delete;

    list_views = g_slist_reverse(list_views);

    for (GSList* iter = list_views; iter; iter = iter->next)
    {
        view = WINTC_ISHEXT_VIEW(iter->data);

        new_view      = TRUE;
        should_delete = FALSE;

        WINTC_LOG_DEBUG(
            "shell: tree - iterate view %s",
            wintc_ishext_view_get_display_name(view)
        );

        // Special handling for the first item
        //   - If there's no iter_path, then we need to create the root node
        //   - If there is an iter path:
        //     - If a node exists but no view, map the view
        //     - If a node exists and already has a view, continue as usual
        //       but do not add another signal to the view
        //
        // FOR YOUR INFORMATION:
        // The case where a node exists and already has a view can happen when:
        //   - An already opened view is opened again (going up one parent)
        //   - A leaf node has gained children (eg. ZIP files opened in the ZIP
        //     shell extension
        //
        if (iter == list_views)
        {
            if (row_ref)
            {
                WinTCIShextView* node_view = NULL;
                GtkTreePath*     tree_path;

                tree_path =
                    gtk_tree_row_reference_get_path(row_ref);

                gtk_tree_model_get_iter(
                    GTK_TREE_MODEL(behaviour->tree_model),
                    &next,
                    tree_path
                );

                gtk_tree_model_get(
                    GTK_TREE_MODEL(behaviour->tree_model),
                    &next,
                    COLUMN_MAPPED_VIEW, &node_view,
                    -1
                );

                gtk_tree_path_free(tree_path);

                if (node_view)
                {
                    WINTC_LOG_DEBUG(
                        "shell: tree - first iter is a mapped view."
                    );

                    // Opposite case here, because we are not mapping this
                    // view, we need to make sure it gets binned if it is not
                    // owned by the browser
                    //
                    // Of course, we can't bin it until we're done with it at
                    // the end of the iteration
                    //
                    if (view != current_view)
                    {
                        should_delete = TRUE;
                    }

                    new_view = FALSE;
                }
                else
                {
                    WINTC_LOG_DEBUG(
                        "shell: tree - first iter is a view item."
                    );

                    if (view == current_view)
                    {
                        g_object_ref(view);
                    }

                    gtk_tree_store_set(
                        behaviour->tree_model,
                        &next,
                        COLUMN_MAPPED_VIEW, view,
                        -1
                    );
                }
            }
            else
            {
                WINTC_LOG_DEBUG("shell: tree - creating top level.");

                hash = wintc_ishext_view_get_unique_hash(view);

                // Create root node
                //
                gtk_tree_store_append(
                    behaviour->tree_model,
                    &next,
                    NULL
                );
                gtk_tree_store_set(
                    behaviour->tree_model,
                    &next,
                    COLUMN_ICON_NAME,   wintc_ishext_view_get_icon_name(view),
                    COLUMN_ENTRY_NAME,  wintc_ishext_view_get_display_name(view),
                    COLUMN_VIEW_HASH,   hash,
                    COLUMN_MAPPED_VIEW, view,
                    -1
                );

                // Map the hash->row
                //
                GtkTreePath* tree_path =
                    gtk_tree_model_get_path(
                        GTK_TREE_MODEL(behaviour->tree_model),
                        &next
                    );

                if (view == current_view)
                {
                    g_object_ref(view);
                }

                g_hash_table_insert(
                    behaviour->map_hash_to_row,
                    GUINT_TO_POINTER(hash),
                    gtk_tree_row_reference_new(
                        GTK_TREE_MODEL(behaviour->tree_model),
                        tree_path
                    )
                );

                gtk_tree_path_free(tree_path);
            }

            last = next; // Copy node iter over
        }

        // If there's a descendant view below this one, create the node
        // immediately
        //
        if (iter->next)
        {
            next_view = WINTC_ISHEXT_VIEW(iter->next->data);

            hash = wintc_ishext_view_get_unique_hash(next_view);

            if (next_view == current_view)
            {
                g_object_ref(next_view);
            }

            gtk_tree_store_append(
                behaviour->tree_model,
                &next,
                &last
            );
            gtk_tree_store_set(
                behaviour->tree_model,
                &next,
                COLUMN_ICON_NAME,   wintc_ishext_view_get_icon_name(next_view),
                COLUMN_ENTRY_NAME,  wintc_ishext_view_get_display_name(next_view),
                COLUMN_VIEW_HASH,   hash,
                COLUMN_MAPPED_VIEW, next_view,
                -1
            );

            // Map the hash->row
            //
            GtkTreePath* tree_path =
                gtk_tree_model_get_path(
                    GTK_TREE_MODEL(behaviour->tree_model),
                    &next
                );

            g_hash_table_insert(
                behaviour->map_hash_to_row,
                GUINT_TO_POINTER(hash),
                gtk_tree_row_reference_new(
                    GTK_TREE_MODEL(behaviour->tree_model),
                    tree_path
                )
            );

            gtk_tree_path_free(tree_path);
        }

        // Watch for items and refresh if this is a new view
        //
        if (new_view)
        {
            g_signal_connect_object(
                view,
                "items-added",
                G_CALLBACK(on_view_items_added),
                behaviour,
                G_CONNECT_DEFAULT
            );
            g_signal_connect_object(
                view,
                "items-removed",
                G_CALLBACK(on_view_items_removed),
                behaviour,
                G_CONNECT_DEFAULT
            );

            // Add the items now - if this is a totally new view this'll just
            // trigger a refresh and the signals will deal with it
            //
            GList* items = wintc_ishext_view_get_items(view);

            wintc_sh_tree_view_behaviour_node_add_items(
                behaviour,
                &last,
                items
            );

            g_list_free(items);
        }

        // Delete the view if needed
        //
        if (should_delete)
        {
            g_object_unref(view);
        }

        // Chain for next iteration
        //
        last = next;
    }

    g_slist_free(list_views);
}

//
// CALLBACKS
//
static void clear_tree_row_reference(
    GtkTreeRowReference* row_ref
)
{
    WinTCIShextView* view = NULL;

    GtkTreeModel* tree_model = gtk_tree_row_reference_get_model(row_ref);
    GtkTreePath*  tree_path  = gtk_tree_row_reference_get_path(row_ref);

    GtkTreeIter row;

    gtk_tree_model_get_iter(
        tree_model,
        &row,
        tree_path
    );

    gtk_tree_model_get(
        tree_model,
        &row,
        COLUMN_MAPPED_VIEW, &view,
        -1
    );

    g_clear_object(&view);
    gtk_tree_path_free(tree_path);
    gtk_tree_row_reference_free(row_ref);
}

static void on_browser_load_changed(
    WinTCShBrowser*         self,
    WinTCShBrowserLoadEvent load_event,
    gpointer                user_data
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(user_data);

    if (load_event != WINTC_SH_BROWSER_LOAD_FINISHED)
    {
        return;
    }

    wintc_sh_tree_view_behaviour_update_view(behaviour, self);
}

static void on_view_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(user_data);

    WINTC_LOG_DEBUG(
        "shell: tree - items added to %p",
        GUINT_TO_POINTER(wintc_ishext_view_get_unique_hash(view))
    );

    // Locate the parent node
    //
    GtkTreeIter          parent;
    GtkTreeRowReference* row_ref;
    GtkTreePath*         tree_path;

    row_ref =
        g_hash_table_lookup(
            behaviour->map_hash_to_row,
            GUINT_TO_POINTER(wintc_ishext_view_get_unique_hash(view))
        );

    tree_path = gtk_tree_row_reference_get_path(row_ref);

    if (
        !gtk_tree_model_get_iter(
            GTK_TREE_MODEL(behaviour->tree_model),
            &parent,
            tree_path
        )
    )
    {
        g_critical("shell: tree - somehow unable to find parent!");
        return;
    }

    gtk_tree_path_free(tree_path);

    // Append the items to the node now
    //
    wintc_sh_tree_view_behaviour_node_add_items(
        behaviour,
        &parent,
        update->data
    );
}

static void on_view_items_removed(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    WinTCShTreeViewBehaviour* behaviour =
        WINTC_SH_TREE_VIEW_BEHAVIOUR(user_data);

    // Track down and bin via the maps
    //
    GtkTreeIter iter;

    for (GList* upd_iter = update->data; upd_iter; upd_iter = upd_iter->next)
    {
        guint item_hash = GPOINTER_TO_UINT(upd_iter->data);

        // Nav to iter
        //
        GtkTreeRowReference* row_ref;
        GtkTreePath*         tree_path;

        row_ref =
            g_hash_table_lookup(
                behaviour->map_hash_to_row,
                GUINT_TO_POINTER(item_hash)
            );

        if (!row_ref)
        {
            continue;
        }

        tree_path =
            gtk_tree_row_reference_get_path(row_ref);

        gtk_tree_model_get_iter(
            GTK_TREE_MODEL(behaviour->tree_model),
            &iter,
            tree_path
        );

        gtk_tree_path_free(tree_path);

        // Delete node and mapping (row ref and view collected up via
        // clear_tree_row_reference())
        //
        gtk_tree_store_remove(
            behaviour->tree_model,
            &iter
        );

        g_hash_table_remove(
            behaviour->map_hash_to_row,
            GUINT_TO_POINTER(item_hash)
        );
    }
}
