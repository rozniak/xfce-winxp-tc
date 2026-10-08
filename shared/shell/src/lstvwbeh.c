#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "../public/browser.h"
#include "../public/fsclipbd.h"
#include "../public/lstvwbeh.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_BROWSER = 1,
    PROP_LIST_VIEW
};

enum
{
    COLUMN_ICON = 0,
    COLUMN_ENTRY_NAME,
    COLUMN_VIEW_HASH,
    N_COLUMNS
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_list_view_behaviour_constructed(
    GObject* object
);
static void wintc_sh_list_view_behaviour_dispose(
    GObject* object
);
static void wintc_sh_list_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_sh_list_view_behaviour_find_child_iter(
    WinTCShListViewBehaviour* behaviour,
    GtkTreeIter*              iter,
    guint                     item_hash
);
static void wintc_sh_list_view_behaviour_insert_item(
    WinTCShListViewBehaviour* behaviour,
    WinTCShextViewItem*       item
);
static void wintc_sh_list_view_behaviour_update_view(
    WinTCShListViewBehaviour* behaviour
);

static gboolean on_list_view_button_press_event(
    WinTCCtlListView* self,
    GdkEventButton*   event,
    gpointer          user_data
);
static void on_list_view_drag_begin(
    GtkWidget*      widget,
    GdkDragContext* context,
    gpointer        user_data
);
static void on_list_view_drag_data_get(
    GtkWidget*        widget,
    GdkDragContext*   context,
    GtkSelectionData* selection_data,
    guint             info,
    guint             time,
    gpointer          user_data
);
static void on_list_view_drag_data_received(
    GtkWidget*        widget,
    GdkDragContext*   context,
    gint              x,
    gint              y,
    GtkSelectionData* selection_data,
    guint             info,
    guint             time,
    gpointer          user_data
);
static gboolean on_list_view_drag_drop(
    GtkWidget*      widget,
    GdkDragContext* context,
    gint            x,
    gint            y,
    guint           time,
    gpointer        user_data
);
static gboolean on_list_view_drag_motion(
    GtkWidget*      widget,
    GdkDragContext* context,
    gint            x,
    gint            y,
    guint           time,
    gpointer        user_data
);
static void on_list_view_item_activated(
    WinTCCtlListView* self,
    GtkTreePath*      path,
    gpointer          user_data
);

static void on_browser_load_changed(
    WinTCShBrowser*          self,
    WinTCShBrowserLoadEvent* load_event,
    gpointer                 user_data
);

static void on_current_view_items_added(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);
static void on_current_view_items_removed(
    WinTCIShextView*           view,
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
);
static void on_current_view_refreshing(
    WinTCIShextView* view,
    gpointer         user_data
);

//
// STATIC DATA
//
static GdkAtom S_ATOM_TEXT_URI_LIST;
static GdkAtom S_ATOM_TEXT_X_WINTC_SHELL_LIST;

static GtkTargetEntry S_DRAG_SOURCES[] = {
    {
        "text/uri-list",
        0,
        WINTC_SHEXT_DND_TARGET_URI_LIST
    },
    {
        "text/x-wintc-shell-list",
        0,
        WINTC_SHEXT_DND_TARGET_X_WINTC_SHELL_LIST
    }
};
static GtkTargetEntry S_DRAG_TARGETS[] = {
    {
        "text/uri-list",
        0,
        WINTC_SHEXT_DND_TARGET_URI_LIST
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShListViewBehaviourClass
{
    GObjectClass __parent__;
};

struct _WinTCShListViewBehaviour
{
    GObject __parent__;

    WinTCShBrowser* browser;

    // UI related
    //
    GtkWidget* list_view;

    gint     drag_x;
    gint     drag_y;
    gboolean drag_motion;

    // View state
    //
    WinTCIShextView* current_view;
    GtkListStore*    list_model;

    gulong sigid_items_added;
    gulong sigid_items_removed;
    gulong sigid_refreshing;

    // Misc. stuff for actions
    //
    WinTCShFSClipboard* fs_clipboard;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShListViewBehaviour,
    wintc_sh_list_view_behaviour,
    G_TYPE_OBJECT
)

static void wintc_sh_list_view_behaviour_class_init(
    WinTCShListViewBehaviourClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_list_view_behaviour_constructed;
    object_class->dispose      = wintc_sh_list_view_behaviour_dispose;
    object_class->set_property = wintc_sh_list_view_behaviour_set_property;

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
        PROP_LIST_VIEW,
        g_param_spec_object(
            "list-view",
            "ListView",
            "The list view to manage.",
            WINTC_TYPE_CTL_LIST_VIEW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );

    S_ATOM_TEXT_URI_LIST =
        gdk_atom_intern_static_string("text/uri-list");
    S_ATOM_TEXT_X_WINTC_SHELL_LIST =
        gdk_atom_intern_static_string("text/x-wintc-shell-list");
}

static void wintc_sh_list_view_behaviour_init(
    WINTC_UNUSED(WinTCShListViewBehaviour* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_list_view_behaviour_constructed(
    GObject* object
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(object);

    if (!behaviour->browser || !behaviour->list_view)
    {
        g_critical("%s", "ShListViewBehaviour: Must have a browser and view!");
        return;
    }

    // Spawn the FS clipboard
    //
    behaviour->fs_clipboard = wintc_sh_fs_clipboard_new();

    // Set up view model
    //
    behaviour->list_model =
        gtk_list_store_new(
            3,
            GDK_TYPE_PIXBUF,
            G_TYPE_STRING,
            G_TYPE_UINT
        );

    // Bind special action states
    //
    // FIXME: Shift to view implementations
    //
    /**
    GAction* action_paste_op =
        g_action_map_lookup_action(G_ACTION_MAP(action_group), "paste-op");

    g_object_bind_property(
        behaviour->fs_clipboard,
        "can-paste",
        action_paste_op,
        "enabled",
        G_BINDING_DEFAULT
    );
    */

    // Attach stuff to view
    //
    wintc_ctl_list_view_set_model(
        WINTC_CTL_LIST_VIEW(behaviour->list_view),
        GTK_TREE_MODEL(behaviour->list_model)
    );
    wintc_ctl_list_view_set_pixbuf_column(
        WINTC_CTL_LIST_VIEW(behaviour->list_view),
        0
    );
    wintc_ctl_list_view_set_text_column(
        WINTC_CTL_LIST_VIEW(behaviour->list_view),
        1
    );

    // Enable drag destination
    //
    wintc_ctl_list_view_enable_drag_dest(
        WINTC_CTL_LIST_VIEW(behaviour->list_view),
        S_DRAG_TARGETS,
        G_N_ELEMENTS(S_DRAG_TARGETS),
        GDK_ACTION_COPY
    );
    wintc_ctl_list_view_enable_drag_source(
        WINTC_CTL_LIST_VIEW(behaviour->list_view),
        NULL,
        0,
        GDK_ACTION_COPY | GDK_ACTION_MOVE
    );

    // Attach signals
    //
    g_signal_connect(
        behaviour->list_view,
        "button-press-event",
        G_CALLBACK(on_list_view_button_press_event),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "drag-begin",
        G_CALLBACK(on_list_view_drag_begin),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "drag-data-get",
        G_CALLBACK(on_list_view_drag_data_get),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "drag-data-received",
        G_CALLBACK(on_list_view_drag_data_received),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "drag-drop",
        G_CALLBACK(on_list_view_drag_drop),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "drag-motion",
        G_CALLBACK(on_list_view_drag_motion),
        behaviour
    );
    g_signal_connect(
        behaviour->list_view,
        "item-activated",
        G_CALLBACK(on_list_view_item_activated),
        behaviour
    );

    // Hook up to shell browser
    //
    wintc_sh_list_view_behaviour_update_view(behaviour);

    g_signal_connect_object(
        behaviour->browser,
        "load-changed",
        G_CALLBACK(on_browser_load_changed),
        behaviour,
        G_CONNECT_DEFAULT
    );

    // Load initial items
    //
    if (behaviour->current_view)
    {
        GList* list_items =
            wintc_ishext_view_get_items(behaviour->current_view);

        for (GList* iter = list_items; iter; iter = iter->next)
        {
            wintc_sh_list_view_behaviour_insert_item(
                behaviour,
                (WinTCShextViewItem*) iter->data
            );
        }

        g_clear_list(&list_items, NULL);
    }

    (G_OBJECT_CLASS(wintc_sh_list_view_behaviour_parent_class))
        ->constructed(object);
}

static void wintc_sh_list_view_behaviour_dispose(
    GObject* object
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(object);

    if (behaviour->list_view)
    {
        wintc_ctl_list_view_set_model(
            WINTC_CTL_LIST_VIEW(behaviour->list_view),
            NULL
        );
    }

    g_clear_object(&(behaviour->current_view));
    g_clear_object(&(behaviour->browser));
    g_clear_object(&(behaviour->list_view));
    g_clear_object(&(behaviour->fs_clipboard));

    (G_OBJECT_CLASS(wintc_sh_list_view_behaviour_parent_class))
        ->dispose(object);
}

static void wintc_sh_list_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(object);

    switch (prop_id)
    {
        case PROP_BROWSER:
            behaviour->browser = g_value_dup_object(value);
            break;

        case PROP_LIST_VIEW:
            behaviour->list_view = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShListViewBehaviour* wintc_sh_list_view_behaviour_new(
    WinTCCtlListView* list_view,
    WinTCShBrowser*   browser
)
{
    return WINTC_SH_LIST_VIEW_BEHAVIOUR(
        g_object_new(
            WINTC_TYPE_SH_LIST_VIEW_BEHAVIOUR,
            "browser",   browser,
            "list-view", list_view,
            NULL
        )
    );
}

GList* wintc_sh_list_view_behaviour_get_selected_items(
    WinTCShListViewBehaviour* behaviour
)
{
    GList*        item_hashes    = NULL;
    GtkTreeModel* model          = wintc_ctl_list_view_get_model(
                                       WINTC_CTL_LIST_VIEW(
                                           behaviour->list_view
                                       )
                                   );
    GList*        selected_items = wintc_ctl_list_view_get_selected_items(
                                       WINTC_CTL_LIST_VIEW(
                                           behaviour->list_view
                                       )
                                   );
    GtkTreeIter   tree_iter;

    for (GList* iter = selected_items; iter; iter = iter->next)
    {
        guint hash;

        gtk_tree_model_get_iter(
            model,
            &tree_iter,
            (GtkTreePath*) iter->data
        );

        gtk_tree_model_get(
            model,
            &tree_iter,
            2, &hash,
            -1
        );

        item_hashes =
            g_list_prepend(
                item_hashes,
                GUINT_TO_POINTER(hash)
            );
    }

    g_list_free_full(
        selected_items,
        (GDestroyNotify) gtk_tree_path_free
    );

    return g_list_reverse(item_hashes);
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_sh_list_view_behaviour_find_child_iter(
    WinTCShListViewBehaviour* behaviour,
    GtkTreeIter*              iter,
    guint                     item_hash
)
{
    gboolean searching =
        gtk_tree_model_iter_children(
            GTK_TREE_MODEL(behaviour->list_model),
            iter,
            NULL
        );

    while (searching)
    {
        guint hash;

        gtk_tree_model_get(
            GTK_TREE_MODEL(behaviour->list_model),
            iter,
            COLUMN_VIEW_HASH, &hash,
            -1
        );

        if (item_hash == hash)
        {
            return TRUE;
        }

        searching =
            gtk_tree_model_iter_next(
                GTK_TREE_MODEL(behaviour->list_model),
                iter
            );
    }

    return FALSE;
}

static void wintc_sh_list_view_behaviour_insert_item(
    WinTCShListViewBehaviour* behaviour,
    WinTCShextViewItem*       item
)
{
    // Load icon
    //
    GtkIconTheme* icon_theme = gtk_icon_theme_get_default();
    GdkPixbuf*    icon       = gtk_icon_theme_load_icon(
                                   icon_theme,
                                   item->icon_name,
                                   32,
                                   GTK_ICON_LOOKUP_FORCE_SIZE |
                                   GTK_ICON_LOOKUP_FORCE_REGULAR,
                                   NULL // FIXME: Error handling
                               );

    // Update model based on if this is a new item or an existing one
    //
    GtkTreeIter iter;

    if (
        !wintc_sh_list_view_behaviour_find_child_iter(
            behaviour,
            &iter,
            item->hash
        )
    )
    {
        GCompareFunc sort_func =
            wintc_ishext_view_get_sort_func(
                behaviour->current_view
            );

        gint item_pos =
            wintc_tree_model_get_insertion_sort_pos(
                GTK_TREE_MODEL(behaviour->list_model),
                NULL,
                COLUMN_VIEW_HASH,
                G_TYPE_UINT,
                sort_func,
                GUINT_TO_POINTER(item->hash)
            );

        gtk_list_store_insert(
            behaviour->list_model,
            &iter,
            item_pos
        );
    }

    gtk_list_store_set(
        behaviour->list_model,
        &iter,
        COLUMN_ICON,       icon,
        COLUMN_ENTRY_NAME, item->display_name,
        COLUMN_VIEW_HASH,  item->hash,
        -1
    );

    // If this is a new item then focus it
    //
    if (item->hint == WINTC_SHEXT_VIEW_ITEM_IS_NEW)
    {
        WINTC_LOG_DEBUG("shell: list view - new item for editing");

        GtkTreePath* tree_path =
            gtk_tree_model_get_path(
                GTK_TREE_MODEL(behaviour->list_model),
                &iter
            );

        gtk_widget_grab_focus(
            behaviour->list_view
        );
        wintc_ctl_list_view_unselect_all(
            WINTC_CTL_LIST_VIEW(behaviour->list_view)
        );
        wintc_ctl_list_view_select_path(
            WINTC_CTL_LIST_VIEW(behaviour->list_view),
            tree_path
        );

        gtk_tree_path_free(tree_path);
    }
}

static void wintc_sh_list_view_behaviour_update_view(
    WinTCShListViewBehaviour* behaviour
)
{
    WinTCIShextView* new_view =
        wintc_sh_browser_get_current_view(behaviour->browser);

    if (behaviour->current_view == new_view)
    {
        return;
    }

    // Detach from old view
    //
    if (behaviour->current_view)
    {
        g_signal_handler_disconnect(
            behaviour->current_view,
            behaviour->sigid_items_added
        );
        g_signal_handler_disconnect(
            behaviour->current_view,
            behaviour->sigid_items_removed
        );
        g_signal_handler_disconnect(
            behaviour->current_view,
            behaviour->sigid_refreshing
        );

        g_clear_object(&(behaviour->current_view));
    }

    // Update the view
    //
    if (!new_view)
    {
        return;
    }

    behaviour->current_view = g_object_ref(new_view);

    behaviour->sigid_items_added =
        g_signal_connect_object(
            behaviour->current_view,
            "items-added",
            G_CALLBACK(on_current_view_items_added),
            behaviour,
            G_CONNECT_DEFAULT
        );
    behaviour->sigid_items_removed =
        g_signal_connect_object(
            behaviour->current_view,
            "items-removed",
            G_CALLBACK(on_current_view_items_removed),
            behaviour,
            G_CONNECT_DEFAULT
        );
    behaviour->sigid_refreshing =
        g_signal_connect_object(
            behaviour->current_view,
            "refreshing",
            G_CALLBACK(on_current_view_refreshing),
            behaviour,
            G_CONNECT_DEFAULT
        );
}

//
// CALLBACKS
//
static gboolean on_list_view_button_press_event(
    WinTCCtlListView* self,
    GdkEventButton*   event,
    gpointer          user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    if (event->button == GDK_BUTTON_SECONDARY)
    {
        WinTCIShextView* view =
            wintc_sh_browser_get_current_view(behaviour->browser);

        // We need to update the hit test target ourselves, since this signal
        // always runs before the icon view handles clicks
        //
        gboolean     found          = FALSE;
        GList*       selected_items = wintc_ctl_list_view_get_selected_items(
                                          self
                                      );
        GtkTreePath* target_item    = wintc_ctl_list_view_get_path_at_pos(
                                          self,
                                          event->x,
                                          event->y
                                      );

        if (target_item)
        {
            // See if the item is in the selected items, otherwise we must
            // select this one
            //
            for (GList* iter = selected_items; iter; iter = iter->next)
            {
                GtkTreePath* check_item = (GtkTreePath*) iter->data;

                if (gtk_tree_path_compare(target_item, check_item) == 0)
                {
                    found = TRUE;
                    break;
                }
            }

            if (!found)
            {
                wintc_ctl_list_view_unselect_all(self);
                wintc_ctl_list_view_select_path(self, target_item);
            }
        }

        g_list_free_full(
            selected_items,
            (GDestroyNotify) gtk_tree_path_free
        );

        // Determine the target...
        //   the view itself?
        //   single view item?
        //   multiple view items?
        //
        GtkWidget*  menu       = NULL;
        GMenuModel* menu_model = NULL;

        if (!target_item) // The view itself
        {
            menu_model =
                wintc_ishext_view_get_operations_for_view(view);

            if (menu_model)
            {
                // We merge the common context menu with this one
                //
                GtkBuilder* builder;
                GMenuModel* menu_model_cmn;
                GMenuModel* menu_model_merged;

                builder =
                    gtk_builder_new_from_resource(
                        "/uk/oddmatics/wintc/shell/menuctx.ui"
                    );

                wintc_lc_builder_preprocess_widget_text(builder);

                menu_model_cmn =
                    G_MENU_MODEL(
                        g_object_ref(
                            gtk_builder_get_object(builder, "menu")
                        )
                    );

                menu_model_merged =
                    wintc_menu_model_merge(
                        menu_model_cmn,
                        menu_model,
                        NULL
                    );

                // Now create the actual popup menu
                //
                menu = gtk_menu_new_from_model(menu_model_merged);

                g_object_unref(builder);
                g_object_unref(menu_model);
                g_object_unref(menu_model_cmn);
            }
        }
        else // Item(s)
        {
            // Always use the menu for the target item, nothing fancy required
            //
            guint         item_hash;
            GtkTreeIter   iter;
            GtkTreeModel* model = wintc_ctl_list_view_get_model(self);

            gtk_tree_model_get_iter(
                model,
                &iter,
                target_item
            );

            gtk_tree_model_get(
                model,
                &iter,
                2, &item_hash,
                -1
            );

            menu_model =
                wintc_ishext_view_get_operations_for_item(
                    view,
                    item_hash
                );

            if (menu_model)
            {
                menu = gtk_menu_new_from_model(menu_model);

                g_object_unref(menu_model);
            }

            gtk_tree_path_free(target_item);
        }

        // Update GActions
        //
        GList* items =
            wintc_sh_list_view_behaviour_get_selected_items(
                behaviour
            );

        GActionGroup* action_group = wintc_ishext_view_get_actions(view);

        wintc_ishext_view_update_actions(view, action_group, items);

        gtk_widget_insert_action_group(
            GTK_WIDGET(self),
            "control",
            action_group
        );

        g_object_unref(action_group);
        g_list_free(items);

        if (menu)
        {
            gtk_menu_attach_to_widget(
                GTK_MENU(menu),
                behaviour->list_view,
                NULL
            );

            gtk_menu_popup_at_pointer(
                GTK_MENU(menu),
                (GdkEvent*) event
            );
        }

        return GDK_EVENT_STOP;
    }

    return GDK_EVENT_PROPAGATE;
}

static void on_list_view_drag_begin(
    GtkWidget* widget,
    WINTC_UNUSED(GdkDragContext* context),
    gpointer   user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    GList* item_hashes =
        wintc_sh_list_view_behaviour_get_selected_items(behaviour);

    // Query what drag targets are actually available for the items in this
    // view
    //
    GtkTargetList* target_list =
        wintc_ctl_list_view_get_source_target_list(
            WINTC_CTL_LIST_VIEW(widget)
        );

    for (gsize i = 0; i < G_N_ELEMENTS(S_DRAG_SOURCES); i++)
    {
        GdkAtom target =
            gdk_atom_intern_static_string(S_DRAG_SOURCES[i].target);

        if (
            wintc_ishext_view_drag_test(
                behaviour->current_view,
                item_hashes,
                S_DRAG_SOURCES[i].info
            )
        )
        {
            gtk_target_list_add(
                target_list,
                target,
                S_DRAG_SOURCES[i].flags,
                S_DRAG_SOURCES[i].info
            );
        }
        else
        {
            gtk_target_list_remove(
                target_list,
                target
            );
        }
    }

    g_list_free(item_hashes);
}

static void on_list_view_drag_data_get(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(GdkDragContext* context),
    GtkSelectionData* selection_data,
    guint             info,
    WINTC_UNUSED(guint time),
    gpointer          user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    GList*  item_hashes =
        wintc_sh_list_view_behaviour_get_selected_items(behaviour);
    gchar** uris_v = NULL;

    if (!item_hashes)
    {
        return;
    }

    // Test it's still valid
    //
    if (
        !wintc_ishext_view_drag_test(
            behaviour->current_view,
            item_hashes,
            info
        )
    )
    {
        goto cleanup;
    }

    // Acquire the data
    //
    GList* uris =
        wintc_ishext_view_drag_execute(
            behaviour->current_view,
            item_hashes,
            info
        );

    uris_v = wintc_list_to_strv(uris, TRUE);

    if (uris)
    {
        gtk_selection_data_set_uris(selection_data, uris_v);
    }

cleanup:
    g_strfreev(uris_v);
    g_list_free(item_hashes);
}

static void on_list_view_drag_data_received(
    GtkWidget*        widget,
    GdkDragContext*   context,
    WINTC_UNUSED(gint x),
    WINTC_UNUSED(gint y),
    GtkSelectionData* selection_data,
    WINTC_UNUSED(guint info),
    guint             time,
    gpointer          user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    GError*      error       = NULL;
    guint        item_hash   = 0;
    GtkTreePath* target_item = NULL;
    gchar**      uris        = NULL;

    gboolean handled = FALSE;

    // Pull URIs
    //
    uris = gtk_selection_data_get_uris(selection_data);

    if (!uris)
    {
        goto cleanup;
    }

    // Test - are we dragging on an item
    //
    target_item =
        wintc_ctl_list_view_get_drop_target(
            WINTC_CTL_LIST_VIEW(widget)
        );

    if (target_item)
    {
        GtkTreeIter iter;

        gtk_tree_model_get_iter(
            GTK_TREE_MODEL(behaviour->list_model),
            &iter,
            target_item
        );

        gtk_tree_model_get(
            GTK_TREE_MODEL(behaviour->list_model),
            &iter,
            COLUMN_VIEW_HASH, &item_hash,
            -1
        );
    }

    // Query the view for the drag
    //
    if (behaviour->drag_motion)
    {
        GdkModifierType mask;

        wintc_widget_get_modifier_mask(
            behaviour->list_view,
            &mask
        );

        if (
            wintc_ishext_view_drop_test(
                behaviour->current_view,
                item_hash,
                (const gchar* const*) uris
            )
        )
        {
            gdk_drag_status(
                context,
                mask & GDK_CONTROL_MASK ?
                    GDK_ACTION_COPY : GDK_ACTION_MOVE,
                time
            );
        }
        else
        {
            gdk_drag_status(context, 0, time);
        }
    }
    else
    {
        gboolean   hint_copy =
            gdk_drag_context_get_selected_action(context) == GDK_ACTION_COPY;
        GtkWindow* wnd =
            wintc_widget_get_toplevel_window(behaviour->list_view);

        if (
            wintc_ishext_view_drop_execute(
                behaviour->current_view,
                wnd,
                item_hash,
                (const gchar* const*) uris,
                hint_copy,
                &error
            )
        )
        {
            gtk_drag_finish(
                context,
                TRUE,
                FALSE,
                time
            );
        }
        else
        {
            gtk_drag_finish(
                context,
                FALSE,
                FALSE,
                time
            );

            wintc_display_error_and_clear(&error, wnd);
        }
    }

    handled = TRUE;

cleanup:
    g_strfreev(uris);
    gtk_tree_path_free(target_item);

    if (!handled)
    {
        if (behaviour->drag_motion)
        {
            gdk_drag_status(context, 0, time);
        }
        else
        {
            gtk_drag_finish(context, FALSE, FALSE, time);
        }
    }
}

static gboolean on_list_view_drag_drop(
    GtkWidget*      widget,
    GdkDragContext* context,
    gint            x,
    gint            y,
    guint           time,
    gpointer        user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    behaviour->drag_x      = x;
    behaviour->drag_y      = y;
    behaviour->drag_motion = FALSE;

    if (
        wintc_ctl_list_view_should_ignore_drop(
            WINTC_CTL_LIST_VIEW(widget)
        )
    )
    {
        gtk_drag_finish(
            context,
            FALSE,
            FALSE,
            0
        );
    }
    else
    {
        gtk_drag_get_data(widget, context, S_ATOM_TEXT_URI_LIST, time);
    }
    
    return TRUE;
}

static gboolean on_list_view_drag_motion(
    GtkWidget*      widget,
    GdkDragContext* context,
    gint            x,
    gint            y,
    guint           time,
    gpointer        user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    behaviour->drag_x      = x;
    behaviour->drag_y      = y;
    behaviour->drag_motion = TRUE;

    gtk_drag_get_data(widget, context, S_ATOM_TEXT_URI_LIST, time);
    
    return TRUE;
}

static void on_list_view_item_activated(
    WinTCCtlListView* self,
    GtkTreePath*      path,
    gpointer          user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    GtkTreeIter   iter;
    GtkTreeModel* model = wintc_ctl_list_view_get_model(self);

    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        GError* error = NULL;
        guint   hash;

        gtk_tree_model_get(
            model,
            &iter,
            2, &hash, // FIXME: Guess we should make the columns public
            -1
        );

        wintc_sh_browser_activate_item(
            behaviour->browser,
            hash,
            &error
        );

        if (error)
        {
            wintc_display_error_and_clear(
                &error,
                wintc_widget_get_toplevel_window(GTK_WIDGET(self))
            );
        }
    }
}

static void on_browser_load_changed(
    WINTC_UNUSED(WinTCShBrowser* self),
    WinTCShBrowserLoadEvent* load_event,
    gpointer                 user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    if (load_event != WINTC_SH_BROWSER_LOAD_STARTED)
    {
        return;
    }

    wintc_sh_list_view_behaviour_update_view(behaviour);
}

static void on_current_view_items_added(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    for (GList* iter = update->data; iter; iter = iter->next)
    {
        wintc_sh_list_view_behaviour_insert_item(
            behaviour,
            (WinTCShextViewItem*) iter->data
        );
    }
}

static void on_current_view_items_removed(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsUpdate* update,
    gpointer                   user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    // FIXME: Inefficient linear search - improve later
    //
    GtkTreeIter iter;

    for (GList* upd_iter = update->data; upd_iter; upd_iter = upd_iter->next)
    {
        guint item_hash = GPOINTER_TO_UINT(upd_iter->data);

        if (
            wintc_sh_list_view_behaviour_find_child_iter(
                behaviour,
                &iter,
                item_hash
            )
        )
        {
            gtk_list_store_remove(
                behaviour->list_model,
                &iter
            );
        }
    }
}

static void on_current_view_refreshing(
    WINTC_UNUSED(WinTCIShextView* view),
    gpointer user_data
)
{
    WinTCShListViewBehaviour* behaviour =
        WINTC_SH_LIST_VIEW_BEHAVIOUR(user_data);

    gtk_list_store_clear(behaviour->list_model);

    wintc_ctl_list_view_reset_layout(
        WINTC_CTL_LIST_VIEW(behaviour->list_view)
    );
}
