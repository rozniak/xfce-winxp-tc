#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "window.h"
#include "classic/ui.h"
#include "welcome/ui.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_PREFER_CLASSIC_LOGON = 1,
    N_PROPERTIES
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCLogonUIWindowClass
{
    GtkWindowClass __parent__;
};

struct _WinTCLogonUIWindow
{
    GtkWindow __parent__;

    GtkWidget* login_ui;
};

//
// FORWARD DECLARATIONS
//
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

    object_class->set_property = wintc_logonui_window_set_property;

    g_object_class_install_property(
        object_class,
        PROP_PREFER_CLASSIC_LOGON,
        g_param_spec_boolean(
            "prefer-classic-logon",
            "PreferClassicLogon",
            "Prefer to use the classic logon user-interface.",
            FALSE,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
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

    //
    // Welcome/Classic UI is decided/init'd in set_property
    //
}

//
// CLASS VIRTUAL METHODS
//
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
        case PROP_PREFER_CLASSIC_LOGON:
            if (g_value_get_boolean(value))
            {
                window->login_ui =
                    wintc_classic_ui_new();
            }
            else
            {
                window->login_ui =
                    wintc_welcome_ui_new();
            }

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_logonui_window_new()
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_LOGONUI_WINDOW,
            "type",                 GTK_WINDOW_TOPLEVEL,
            "decorated",            FALSE,
            "resizable",            FALSE,
            "type-hint",            GDK_WINDOW_TYPE_HINT_DESKTOP,
            "prefer-classic-logon", FALSE,
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

