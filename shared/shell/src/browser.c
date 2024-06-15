#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../public/browser.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_SHEXT_HOST = 1
};

enum
{
    SIGNAL_LOAD_CHANGED = 0,
    N_SIGNALS
};

enum
{
    COLUMN_VIEWITEM = 0,
    COLUMN_ICON,
    COLUMN_DISPLAY_NAME,
    N_COLUMNS
};

//
// STATIC DATA
//
static gint wintc_sh_browser_signals[N_SIGNALS] = { 0 };

//
// FORWARD DECLARATIONS
//
static void wintc_sh_browser_dispose(
    GObject* object
);
static void wintc_sh_browser_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_browser_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void on_current_view_items_added(
    WinTCIShextView*              view,
    WinTCShextViewItemsAddedData* event_data,
    gpointer                      user_data
);
static void on_current_view_items_removed(
    WinTCIShextView*     view,
    WinTCShextViewItem** items,
    gpointer             user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShBrowserClass
{
    GObjectClass __parent__;
};

struct _WinTCShBrowser
{
    GObject __parent__;

    WinTCShextHost* shext_host;

    // Browser state
    //
    WinTCIShextView* current_view;
    GtkListStore*    view_model;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShBrowser,
    wintc_sh_browser,
    G_TYPE_OBJECT
)

static void wintc_sh_browser_class_init(
    WinTCShBrowserClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose      = wintc_sh_browser_dispose;
    object_class->get_property = wintc_sh_browser_get_property;
    object_class->set_property = wintc_sh_browser_set_property;

    g_object_class_install_property(
        object_class,
        PROP_SHEXT_HOST,
        g_param_spec_object(
            "shext-host",
            "ShextHost",
            "The shell extension host object to use.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );

    wintc_sh_browser_signals[SIGNAL_LOAD_CHANGED] =
        g_signal_new(
            "load-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__INT,
            G_TYPE_NONE,
            1,
            G_TYPE_INT
        );
}

static void wintc_sh_browser_init(
    WinTCShBrowser* self
)
{
    // Set up view model
    //
    self->view_model =
        gtk_list_store_new(
            3,
            G_TYPE_POINTER,
            GDK_TYPE_PIXBUF,
            G_TYPE_STRING
        );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_browser_dispose(
    GObject* object
)
{
    WinTCShBrowser* browser = WINTC_SH_BROWSER(object);

    g_clear_object(&(browser->shext_host));
    g_clear_object(&(browser->current_view));

    (G_OBJECT_CLASS(wintc_sh_browser_parent_class))
        ->dispose(object);
}

static void wintc_sh_browser_get_property(
    GObject*    object,
    guint       prop_id,
    WINTC_UNUSED(GValue* value),
    GParamSpec* pspec
)
{
    switch (prop_id)
    {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_sh_browser_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShBrowser* browser = WINTC_SH_BROWSER(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            browser->shext_host = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShBrowser* wintc_sh_browser_new(
    WinTCShextHost* shext_host
)
{
    return WINTC_SH_BROWSER(
        g_object_new(
            WINTC_TYPE_SH_BROWSER,
            "shext-host", shext_host,
            NULL
        )
    );
}

gboolean wintc_sh_browser_activate_item(
    WinTCShBrowser*     browser,
    WinTCShextViewItem* item,
    GError**            error
)
{
    GError*            local_error = NULL;
    WinTCShextPathInfo path_info   = { 0 };

    WINTC_SAFE_REF_CLEAR(error);

    if (!browser->current_view)
    {
        g_critical("%s", "shell: browser - activate item with no view");
        return FALSE;
    }

    // Attempt to activate the item in the view
    //
    if (
        !wintc_ishext_view_activate_item(
            browser->current_view,
            item,
            &path_info,
            &local_error
        )
    )
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    // It's possible the item launched an executable rather than it being a
    // location change
    //
    if (!path_info.base_path)
    {
        return TRUE;
    }

    // Navigate to the path
    //
    gboolean success =
        wintc_sh_browser_set_location(
            browser,
            &path_info,
            &local_error
        );

    if (!success)
    {
        g_propagate_error(error, local_error);
    }

    wintc_shext_path_info_free_data(&path_info);

    return success;
}

gboolean wintc_sh_browser_can_navigate_to_parent(
    WinTCShBrowser* browser
)
{
    if (!browser->current_view)
    {
        g_critical("%s", "shell: nav to parent - no view");
        return FALSE;
    }

    return wintc_ishext_view_has_parent(browser->current_view);
}

void wintc_sh_browser_get_location(
    WinTCShBrowser*     browser,
    WinTCShextPathInfo* path_info
)
{
    if (!browser->current_view)
    {
        return;
    }

    wintc_ishext_view_get_path(
        browser->current_view,
        path_info
    );
}

GtkTreeModel* wintc_sh_browser_get_model(
    WinTCShBrowser* browser
)
{
    return GTK_TREE_MODEL(browser->view_model);
}

const gchar* wintc_sh_browser_get_view_display_name(
    WinTCShBrowser* browser
)
{
    return wintc_ishext_view_get_display_name(browser->current_view);
}

void wintc_sh_browser_navigate_to_parent(
    WinTCShBrowser* browser
)
{
    WinTCShextPathInfo path_info = { 0 };

    if (!wintc_sh_browser_can_navigate_to_parent(browser))
    {
        g_critical("%s", "shell: browser can't nav to parent");
        return;
    }

    wintc_ishext_view_get_parent_path(
        browser->current_view,
        &path_info
    );

    wintc_sh_browser_set_location(browser, &path_info, NULL);

    wintc_shext_path_info_free_data(&path_info);
}

void wintc_sh_browser_refresh(
    WinTCShBrowser* browser
)
{
    if (!browser->current_view)
    {
        g_critical("%s", "shell: browser can't refresh, no view");
        return;
    }

    gtk_list_store_clear(browser->view_model);
    wintc_ishext_view_refresh_items(browser->current_view);
}

gboolean wintc_sh_browser_set_location(
    WinTCShBrowser*           browser,
    const WinTCShextPathInfo* path_info,
    GError**                  error
)
{
    WinTCIShextView* new_view = NULL;

    new_view =
        wintc_shext_host_get_view_for_path(
            browser->shext_host,
            path_info,
            error
        );

    if (!new_view)
    {
        return FALSE;
    }

    // Update the view
    //
    g_clear_object(&(browser->current_view));
    browser->current_view = new_view;

    g_signal_connect(
        browser->current_view,
        "items-added",
        G_CALLBACK(on_current_view_items_added),
        browser
    );
    g_signal_connect(
        browser->current_view,
        "items-removed",
        G_CALLBACK(on_current_view_items_removed),
        browser
    );

    // Notify that we're loading...
    //
    g_signal_emit(
        browser,
        wintc_sh_browser_signals[SIGNAL_LOAD_CHANGED],
        0,
        WINTC_SH_BROWSER_LOAD_STARTED
    );

    // Do the actual load
    //
    wintc_sh_browser_refresh(browser);

    return TRUE;
}

//
// CALLBACKS
//
static void on_current_view_items_added(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItemsAddedData* event_data,
    gpointer                      user_data
)
{
    WinTCShBrowser* browser = WINTC_SH_BROWSER(user_data);

    for (int i = 0; i < event_data->num_items; i++)
    {
        WinTCShextViewItem* item = &(event_data->items[i]);

        // Load icon
        //
        GtkIconTheme* icon_theme = gtk_icon_theme_get_default();
        GdkPixbuf*    icon       = gtk_icon_theme_load_icon(
                                       icon_theme,
                                       item->icon_name,
                                       32,
                                       GTK_ICON_LOOKUP_FORCE_SIZE,
                                       NULL // FIXME: Error handling
                                   );

        // Push to model
        //
        GtkTreeIter iter;

        gtk_list_store_append(browser->view_model, &iter);
        gtk_list_store_set(
            browser->view_model,
            &iter,
            COLUMN_VIEWITEM,     item,
            COLUMN_ICON,         icon,
            COLUMN_DISPLAY_NAME, item->display_name,
            -1
        );
    }

    // Check if done
    //
    if (event_data->done)
    {
        WINTC_LOG_DEBUG("%s", "shell: current view finished refreshing");

        g_signal_emit(
            browser,
            wintc_sh_browser_signals[SIGNAL_LOAD_CHANGED],
            0,
            WINTC_SH_BROWSER_LOAD_FINISHED
        );
    }
}

static void on_current_view_items_removed(
    WINTC_UNUSED(WinTCIShextView* view),
    WinTCShextViewItem** items,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCShextViewItem* p_item = items[0];

    // FIXME: Proper implementation later, just print the names for now
    //
    while (p_item)
    {
        g_message("Item removed: %s", p_item->display_name);
        p_item++;
    }
}
