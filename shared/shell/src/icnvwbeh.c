#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shlang.h>

#include "../public/browser.h"
#include "../public/icnvwbeh.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_BROWSER = 1,
    PROP_ICON_VIEW
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_icon_view_behaviour_constructed(
    GObject* object
);
static void wintc_sh_icon_view_behaviour_dispose(
    GObject* object
);
static void wintc_sh_icon_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void action_view_operation(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static gboolean on_icon_view_button_press_event(
    GtkIconView*    self,
    GdkEventButton* event,
    gpointer        user_data
);
static void on_icon_view_item_activated(
    GtkIconView* self,
    GtkTreePath* path,
    gpointer     user_data
);

//
// STATIC DATA
//
static GSimpleAction* s_action_noop = NULL;

static GActionEntry s_actions[] = {
    {
        .name           = "view-op",
        .activate       = action_view_operation,
        .parameter_type = "i",
        .state          = NULL,
        .change_state   = NULL
    }
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShIconViewBehaviourClass
{
    GObjectClass __parent__;
};

struct _WinTCShIconViewBehaviour
{
    GObject __parent__;

    WinTCShBrowser* browser;
    GtkWidget*      icon_view;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShIconViewBehaviour,
    wintc_sh_icon_view_behaviour,
    G_TYPE_OBJECT
)

static void wintc_sh_icon_view_behaviour_class_init(
    WinTCShIconViewBehaviourClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_icon_view_behaviour_constructed;
    object_class->dispose      = wintc_sh_icon_view_behaviour_dispose;
    object_class->set_property = wintc_sh_icon_view_behaviour_set_property;

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
        PROP_ICON_VIEW,
        g_param_spec_object(
            "icon-view",
            "IconView",
            "The icon view to manage.",
            GTK_TYPE_ICON_VIEW,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_sh_icon_view_behaviour_init(
    WINTC_UNUSED(WinTCShIconViewBehaviour* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_icon_view_behaviour_constructed(
    GObject* object
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(object);

    if (!behaviour->browser || !behaviour->icon_view)
    {
        g_critical("%s", "ShIconViewBehaviour: Must have a browser and view!");
        return;
    }

    // Define GActions
    //
    GSimpleActionGroup* action_group = g_simple_action_group_new();

    if (!s_action_noop)
    {
        s_action_noop =
            g_simple_action_new("no-op", NULL);

        g_simple_action_set_enabled(
            s_action_noop,
            FALSE
        );
    }

    g_action_map_add_action_entries(
        G_ACTION_MAP(action_group),
        s_actions,
        G_N_ELEMENTS(s_actions),
        behaviour
    );
    g_action_map_add_action(
        G_ACTION_MAP(action_group),
        G_ACTION(s_action_noop)
    );

    gtk_widget_insert_action_group(
        behaviour->icon_view,
        "control",
        G_ACTION_GROUP(action_group)
    );

    g_object_unref(action_group);

    // Attach stuff to view
    //
    gtk_icon_view_set_model(
        GTK_ICON_VIEW(behaviour->icon_view),
        wintc_sh_browser_get_model(behaviour->browser)
    );
    gtk_icon_view_set_pixbuf_column(
        GTK_ICON_VIEW(behaviour->icon_view),
        0
    );
    gtk_icon_view_set_text_column(
        GTK_ICON_VIEW(behaviour->icon_view),
        1
    );

    gtk_icon_view_set_selection_mode(
        GTK_ICON_VIEW(behaviour->icon_view),
        GTK_SELECTION_MULTIPLE
    );

    // Attach signals
    //
    g_signal_connect(
        behaviour->icon_view,
        "button-press-event",
        G_CALLBACK(on_icon_view_button_press_event),
        behaviour
    );
    g_signal_connect(
        behaviour->icon_view,
        "item-activated",
        G_CALLBACK(on_icon_view_item_activated),
        behaviour
    );

    (G_OBJECT_CLASS(wintc_sh_icon_view_behaviour_parent_class))
        ->constructed(object);
}

static void wintc_sh_icon_view_behaviour_dispose(
    GObject* object
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(object);

    // Cheeky? Deselect items from the icon view, otherwise it might throw
    // GTK-Critical errors during destruction (?)
    //
    gtk_icon_view_unselect_all(
        GTK_ICON_VIEW(behaviour->icon_view)
    );

    g_clear_object(&(behaviour->browser));
    g_clear_object(&(behaviour->icon_view));

    (G_OBJECT_CLASS(wintc_sh_icon_view_behaviour_parent_class))
        ->dispose(object);
}

static void wintc_sh_icon_view_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(object);

    switch (prop_id)
    {
        case PROP_BROWSER:
            behaviour->browser = g_value_dup_object(value);
            break;

        case PROP_ICON_VIEW:
            behaviour->icon_view = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShIconViewBehaviour* wintc_sh_icon_view_behaviour_new(
    GtkIconView*    icon_view,
    WinTCShBrowser* browser
)
{
    return WINTC_SH_ICON_VIEW_BEHAVIOUR(
        g_object_new(
            WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR,
            "browser",   browser,
            "icon-view", icon_view,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void action_view_operation(
    WINTC_UNUSED(GSimpleAction* action),
    GVariant*      parameter,
    gpointer       user_data
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(user_data);

    WINTC_LOG_DEBUG("op selected: %d", g_variant_get_int32(parameter));

    // Prepare items - need to convert the selected items to their hashes
    //
    GList*        item_hashes    = NULL;
    GtkTreeModel* model          = gtk_icon_view_get_model(
                                       GTK_ICON_VIEW(behaviour->icon_view)
                                   );
    GList*        selected_items = gtk_icon_view_get_selected_items(
                                       GTK_ICON_VIEW(behaviour->icon_view)
                                   );
    GtkTreeIter tree_iter;

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

        WINTC_LOG_DEBUG("shell: icnvw - append item to op: %u", hash);

        item_hashes =
            g_list_prepend(
                item_hashes,
                GUINT_TO_POINTER(hash)
            );
    }

    item_hashes = g_list_reverse(item_hashes);

    g_list_free_full(
        selected_items,
        (GDestroyNotify) gtk_tree_path_free
    );

    // Ask the view to spawn the operation
    //
    GError*              error        = NULL;
    WinTCShextOperation* operation;
    gint                 operation_id = g_variant_get_int32(parameter);
    WinTCIShextView*     view         = wintc_sh_browser_get_current_view(
                                            behaviour->browser
                                        );

    operation =
        wintc_ishext_view_spawn_operation(
            view,
            operation_id,
            item_hashes, // Ownership transferred
            &error
        );

//    g_list_free(item_hashes);

    if (!operation)
    {
        wintc_display_error_and_clear(&error);
        return;
    }

    // Execute!
    //
    GtkWidget* toplevel = gtk_widget_get_toplevel(behaviour->icon_view);
    GtkWindow* wnd      = NULL;

    if (GTK_IS_WINDOW(toplevel))
    {
        wnd = GTK_WINDOW(toplevel);
    }

    if (!(operation->func) (view, operation, wnd, &error))
    {
        wintc_display_error_and_clear(&error);
    }

    g_free(operation);
}

static gboolean on_icon_view_button_press_event(
    GtkIconView*    self,
    GdkEventButton* event,
    gpointer        user_data
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(user_data);

    if (event->button == GDK_BUTTON_SECONDARY)
    {
        // We need to update the hit test target ourselves, since this signal
        // always runs before the icon view handles clicks
        //
        gboolean     found          = FALSE;
        GList*       selected_items = gtk_icon_view_get_selected_items(self);
        GtkTreePath* target_item    = gtk_icon_view_get_path_at_pos(
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
                gtk_icon_view_unselect_all(self);
                gtk_icon_view_select_path(self, target_item);
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
                wintc_ishext_view_get_operations_for_view(
                    wintc_sh_browser_get_current_view(
                        behaviour->browser
                    )
                );

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
            GtkTreeModel* model = gtk_icon_view_get_model(self);

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
                    wintc_sh_browser_get_current_view(behaviour->browser),
                    item_hash
                );

            if (menu_model)
            {
                menu = gtk_menu_new_from_model(menu_model);

                g_object_unref(menu_model);
            }

            gtk_tree_path_free(target_item);
        }

        if (menu)
        {
            gtk_menu_attach_to_widget(
                GTK_MENU(menu),
                behaviour->icon_view,
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

static void on_icon_view_item_activated(
    GtkIconView* self,
    GtkTreePath* path,
    gpointer     user_data
)
{
    WinTCShIconViewBehaviour* behaviour =
        WINTC_SH_ICON_VIEW_BEHAVIOUR(user_data);

    GtkTreeIter   iter;
    GtkTreeModel* model = gtk_icon_view_get_model(self);

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
            wintc_display_error_and_clear(&error);
        }
    }
}
