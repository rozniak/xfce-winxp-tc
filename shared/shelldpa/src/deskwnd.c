#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/api.h"
#include "../public/deskwnd.h"
#include "dll/layersh.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_MONITOR = 1
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCDpaDesktopWindowPrivate
{
    GdkMonitor* monitor;
} WinTCDpaDesktopWindowPrivate;

//
// FORWARD DECLARATIONS
//
static void wintc_dpa_desktop_window_constructed(
    GObject* object
);
static void wintc_dpa_desktop_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_dpa_desktop_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void window_setup_wayland(
    WinTCDpaDesktopWindow* wnd
);
static void window_setup_x11(
    WinTCDpaDesktopWindow* wnd
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCDpaDesktopWindow,
    wintc_dpa_desktop_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_ADD_PRIVATE(WinTCDpaDesktopWindow)
)

static void wintc_dpa_desktop_window_class_init(
    WinTCDpaDesktopWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_dpa_desktop_window_constructed;
    object_class->get_property = wintc_dpa_desktop_window_get_property;
    object_class->set_property = wintc_dpa_desktop_window_set_property;

    g_object_class_install_property(
        object_class,
        PROP_MONITOR,
        g_param_spec_object(
            "monitor",
            "Monitor",
            "The monitor to map to.",
            GDK_TYPE_MONITOR,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_dpa_desktop_window_init(
    WINTC_UNUSED(WinTCDpaDesktopWindow* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_dpa_desktop_window_constructed(
    GObject* object
)
{
    WinTCDpaDesktopWindow* wnd = WINTC_DPA_DESKTOP_WINDOW(object);

    if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_WAYLAND)
    {
        window_setup_wayland(wnd);
    }
    else
    {
        window_setup_x11(wnd);
    }
}

static void wintc_dpa_desktop_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCDpaDesktopWindowPrivate* priv =
        wintc_dpa_desktop_window_get_instance_private(
            WINTC_DPA_DESKTOP_WINDOW(object)
        );

    switch (prop_id)
    {
        case PROP_MONITOR:
            g_value_set_object(value, priv->monitor);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_dpa_desktop_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCDpaDesktopWindowPrivate* priv =
        wintc_dpa_desktop_window_get_instance_private(
            WINTC_DPA_DESKTOP_WINDOW(object)
        );

    switch (prop_id)
    {
        case PROP_MONITOR:
            priv->monitor = g_value_get_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PRIVATE FUNCTIONS
//
static void window_setup_wayland(
    WinTCDpaDesktopWindow* wnd
)
{
    WinTCDpaDesktopWindowPrivate* priv =
        wintc_dpa_desktop_window_get_instance_private(wnd);

    GtkWindow* window = GTK_WINDOW(wnd);

    p_gtk_layer_init_for_window(window);
    p_gtk_layer_set_layer(window, GTK_LAYER_SHELL_LAYER_BACKGROUND);
    p_gtk_layer_set_monitor(window, priv->monitor);

    for (int edge = 0; edge <= GTK_LAYER_SHELL_EDGE_BOTTOM; edge++)
    {
        p_gtk_layer_set_anchor(window, edge, TRUE);
        p_gtk_layer_set_margin(window, edge, 0);
    }

    p_gtk_layer_set_namespace(window, "desktop");
}

static void window_setup_x11(
    WinTCDpaDesktopWindow* wnd
)
{
    WinTCDpaDesktopWindowPrivate* priv =
        wintc_dpa_desktop_window_get_instance_private(wnd);

    GdkRectangle geometry;

    gdk_monitor_get_geometry(priv->monitor, &geometry);

    gtk_window_move(GTK_WINDOW(wnd), geometry.x, geometry.y);
    gtk_window_set_type_hint(GTK_WINDOW(wnd), GDK_WINDOW_TYPE_HINT_DESKTOP);
    gtk_widget_set_size_request(
        GTK_WIDGET(wnd),
        geometry.width,
        geometry.height
    );
}
