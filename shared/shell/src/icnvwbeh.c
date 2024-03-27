#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

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

static void on_icon_view_item_activated(
    GtkIconView* self,
    GtkTreePath* path,
    gpointer     user_data
);

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

    // Attach stuff to view
    //
    gtk_icon_view_set_model(
        GTK_ICON_VIEW(behaviour->icon_view),
        wintc_sh_browser_get_model(behaviour->browser)
    );
    gtk_icon_view_set_pixbuf_column(
        GTK_ICON_VIEW(behaviour->icon_view),
        1
    );
    gtk_icon_view_set_text_column(
        GTK_ICON_VIEW(behaviour->icon_view),
        2
    );

    // Attach signals
    //
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
        GError*             error = NULL;
        WinTCShextViewItem* item  = NULL;

        gtk_tree_model_get(
            model,
            &iter,
            0, &item, // FIXME: Guess we should make the columns public
            -1
        );

        wintc_sh_browser_activate_item(
            behaviour->browser,
            item,
            &error
        );

        if (error)
        {
            wintc_display_error_and_clear(&error);
        }
    }
}
