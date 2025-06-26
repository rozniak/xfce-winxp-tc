#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "settings.h"
#include "window.h"
#include "classic/ui.h"
#include "welcome/ui.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SETTINGS,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_logonui_window_constructed(
    GObject* object
);
static void wintc_logonui_window_dispose(
    GObject* object
);
static void wintc_logonui_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);
static void on_window_realize(
    GtkWidget* self,
    gpointer   user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_logonui_window_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCLogonUIWindow
{
    GtkWindow __parent__;

    GtkWidget*            login_ui;
    WinTCLogonUISettings* settings;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCLogonUIWindow,
    wintc_logonui_window,
    GTK_TYPE_WINDOW
)

static void wintc_logonui_window_class_init(
    WinTCLogonUIWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_logonui_window_constructed;
    object_class->dispose      = wintc_logonui_window_dispose;
    object_class->set_property = wintc_logonui_window_set_property;

    wintc_logonui_window_properties[PROP_SETTINGS] =
        g_param_spec_object(
            "settings",
            "Settings",
            "The object for providing configuration settings.",
            WINTC_TYPE_LOGONUI_SETTINGS,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_logonui_window_properties
    );
}

static void wintc_logonui_window_init(
    WinTCLogonUIWindow* self
)
{
    // Set up window
    //
    GdkDisplay*  display = gdk_display_get_default();
    GdkRectangle geometry;
    GdkMonitor*  monitor = gdk_display_get_primary_monitor(display);

    gdk_monitor_get_geometry(monitor, &geometry);

    gtk_widget_set_size_request(
        GTK_WIDGET(self),
        geometry.width,
        geometry.height
    );
    gtk_window_move(
        GTK_WINDOW(self),
        geometry.x,
        geometry.y
    );

    // We delay some stuff until realize/map events:
    //   - we do not add the welcome/classic UI widget until we
    //     hit the 'map-event' signal, this ensures that any child
    //     windows are created after this window
    //   - delay cursor / other init until we realize the GDK
    //     resources for the window
    //
    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );
    g_signal_connect(
        self,
        "realize",
        G_CALLBACK(on_window_realize),
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_logonui_window_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_logonui_window_parent_class))
        ->constructed(object);

    WinTCLogonUIWindow* window = WINTC_LOGONUI_WINDOW(object);

    // Create the logon session
    //
    WinTCGinaLogonSession* logon_session = wintc_gina_logon_session_new();

    wintc_gina_logon_session_set_preferred_session(
        logon_session,
        wintc_logonui_settings_get_session(window->settings)
    );

    // FIXME: Server SKUs do not have welcome screen
    //
    if (wintc_logonui_settings_get_use_classic_logon(window->settings))
    {
        window->login_ui =
            wintc_classic_ui_new(
                logon_session
            );
    }
    else
    {
        window->login_ui =
            wintc_welcome_ui_new(
                logon_session
            );
    }

    g_object_unref(logon_session);
}

static void wintc_logonui_window_dispose(
    GObject* object
)
{
    WinTCLogonUIWindow* window = WINTC_LOGONUI_WINDOW(object);

    g_clear_object(&(window->settings));

    (G_OBJECT_CLASS(wintc_logonui_window_parent_class))
        ->dispose(object);
}

static void wintc_logonui_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCLogonUIWindow* window = WINTC_LOGONUI_WINDOW(object);

    switch (prop_id)
    {
        case PROP_SETTINGS:
            window->settings = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_logonui_window_new(
    WinTCLogonUISettings* settings
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_LOGONUI_WINDOW,
            "type",                 GTK_WINDOW_TOPLEVEL,
            "decorated",            FALSE,
            "resizable",            FALSE,
            "type-hint",            GDK_WINDOW_TYPE_HINT_DESKTOP,
            "settings",             settings,
            NULL
        )
    );
}

//
// CALLBACKS
//
static gboolean on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny* event),
    WINTC_UNUSED(gpointer     user_data)
)
{
    WinTCLogonUIWindow* window = WINTC_LOGONUI_WINDOW(self);

    gtk_container_add(
        GTK_CONTAINER(self),
        window->login_ui
    );
    gtk_widget_show_all(window->login_ui);

    return TRUE;
}

static void on_window_realize(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    gdk_window_set_cursor(
        gtk_widget_get_window(self),
        gdk_cursor_new_for_display(
            gdk_display_get_default(),
            GDK_LEFT_PTR
        )
    );
}

