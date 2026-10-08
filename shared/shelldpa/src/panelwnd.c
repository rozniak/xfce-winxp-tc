#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/api.h"
#include "../public/panelwnd.h"
#include "dll/layersh.h"

#define GEO_BOTTOM(geo) (geo.y + geo.height)
#define GEO_RIGHT(geo)  (geo.x + geo.width)
#define VALID_GRAVITY(grav) ((grav % 2 == 0) && (grav != GDK_GRAVITY_STATIC))

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_DOCK_SIDE,
    N_PROPERTIES
};

//
// STRUCTURE DEFINITIONS
//
struct X11Struts
{
    gulong left;
    gulong right;
    gulong top;
    gulong bottom;

    gulong left_start_y;
    gulong left_end_y;

    gulong right_start_y;
    gulong right_end_y;

    gulong top_start_x;
    gulong top_end_x;

    gulong bottom_start_x;
    gulong bottom_end_x;
};

//
// FORWARD DECLARATIONS
//
static void wintc_dpa_panel_window_constructed(
    GObject* object
);
static void wintc_dpa_panel_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_dpa_panel_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void update_wayland_anchor(
    WinTCDpaPanelWindow* wnd
);
static void update_x11_struts(
    WinTCDpaPanelWindow* wnd
);
static void window_setup_wayland(
    WinTCDpaPanelWindow* wnd
);
static void window_setup_x11(
    WinTCDpaPanelWindow* wnd
);

static void on_monitor_notify_geometry(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_window_realize_x11(
    GtkWidget* self,
    gpointer   user_data
);
static void on_window_size_allocate_x11(
    GtkWidget*    widget,
    GdkRectangle* allocation,
    gpointer      user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_dpa_panel_window_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCDpaPanelWindowPrivate
{
    GdkGravity dock_side;
    gint       last_size;
} WinTCDpaPanelWindowPrivate;

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(
    WinTCDpaPanelWindow,
    wintc_dpa_panel_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_dpa_panel_window_class_init(
    WinTCDpaPanelWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_dpa_panel_window_constructed;
    object_class->get_property = wintc_dpa_panel_window_get_property;
    object_class->set_property = wintc_dpa_panel_window_set_property;

    wintc_dpa_panel_window_properties[PROP_DOCK_SIDE] =
        g_param_spec_int(
            "dock-side",
            "DockSide",
            "The side of the screen that the window should be docked.",
            0,
            GDK_GRAVITY_SOUTH,
            GDK_GRAVITY_SOUTH,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_EXPLICIT_NOTIFY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_dpa_panel_window_properties
    );
}

static void wintc_dpa_panel_window_init(
    WINTC_UNUSED(WinTCDpaPanelWindow* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_dpa_panel_window_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_dpa_panel_window_parent_class))
        ->constructed(object);

    WinTCDpaPanelWindow* wnd = WINTC_DPA_PANEL_WINDOW(object);

    if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_WAYLAND)
    {
        window_setup_wayland(wnd);
    }
    else // X11
    {
        window_setup_x11(wnd);
    }
}

static void wintc_dpa_panel_window_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(
            WINTC_DPA_PANEL_WINDOW(object)
        );

    switch (prop_id)
    {
        case PROP_DOCK_SIDE:
            g_value_set_int(value, priv->dock_side);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_dpa_panel_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCDpaPanelWindow* wnd = WINTC_DPA_PANEL_WINDOW(object);

    switch (prop_id)
    {
        case PROP_DOCK_SIDE:
            wintc_dpa_panel_window_set_dock_side(
                wnd,
                g_value_get_int(value)
            );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
GdkGravity wintc_dpa_panel_window_get_dock_side(
    WinTCDpaPanelWindow* wnd
)
{
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(
            WINTC_DPA_PANEL_WINDOW(wnd)
        );

    return priv->dock_side;
}

void wintc_dpa_panel_window_set_dock_side(
    WinTCDpaPanelWindow* wnd,
    GdkGravity           dock_side
)
{
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(
            WINTC_DPA_PANEL_WINDOW(wnd)
        );

    if (priv->dock_side == dock_side)
    {
        return;
    }

    if (VALID_GRAVITY(dock_side))
    {
        priv->dock_side = dock_side;

        if (wintc_get_display_protocol_in_use() == WINTC_DISPPROTO_WAYLAND)
        {
            update_wayland_anchor(wnd);
        }
        else // X11
        {
            update_x11_struts(wnd);
        }

        g_object_notify_by_pspec(
            G_OBJECT(wnd),
            wintc_dpa_panel_window_properties[PROP_DOCK_SIDE]
        );
    }
    else
    {
        g_critical("dpa: invalid gravity for panel: %d", dock_side);
    }
}

//
// PRIVATE FUNCTIONS
//
static void update_wayland_anchor(
    WinTCDpaPanelWindow* wnd
)
{
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(
            WINTC_DPA_PANEL_WINDOW(wnd)
        );

    gboolean anchors[4] = { TRUE, TRUE, TRUE, TRUE };

    switch (priv->dock_side)
    {
        case GDK_GRAVITY_NORTH:
            anchors[GTK_LAYER_SHELL_EDGE_BOTTOM] = FALSE;
            break;

        case GDK_GRAVITY_WEST:
            anchors[GTK_LAYER_SHELL_EDGE_RIGHT] = FALSE;
            break;

        case GDK_GRAVITY_EAST:
            anchors[GTK_LAYER_SHELL_EDGE_LEFT] = FALSE;
            break;

        case GDK_GRAVITY_SOUTH:
            anchors[GTK_LAYER_SHELL_EDGE_TOP] = FALSE;
            break;

        default: break;
    }

    for (gsize i = 0; i < G_N_ELEMENTS(anchors); i++)
    {
        p_gtk_layer_set_anchor(GTK_WINDOW(wnd), i, anchors[i]);
    }
}

static void update_x11_struts(
    WinTCDpaPanelWindow* wnd
)
{
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(
            WINTC_DPA_PANEL_WINDOW(wnd)
        );

    if (!gtk_widget_get_realized(GTK_WIDGET(wnd)))
    {
        return;
    }

    GdkAtom cardinal_atom =
        gdk_atom_intern_static_string("CARDINAL");
    GdkAtom net_wm_strut_partial_atom =
        gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");

    // Work out the extreme edge
    //
    GdkDisplay*  display       = gdk_display_get_default();
    GdkRectangle geometry;
    GdkMonitor*  monitor       = NULL;
    gint         monitor_count = gdk_display_get_n_monitors(display);

    gint screen_edge = 0;

    for (int i = 0; i < monitor_count; i++)
    {
        GdkMonitor* monitor_i = gdk_display_get_monitor(display, i);

        gdk_monitor_get_geometry(monitor_i, &geometry);

        if (gdk_monitor_is_primary(monitor_i))
        {
            monitor = monitor_i;
        }

        // Update screen edge
        //
        gint monitor_edge = 0;

        switch (priv->dock_side)
        {
            case GDK_GRAVITY_NORTH:
                monitor_edge = geometry.y;

                if (monitor_edge < screen_edge)
                {
                    screen_edge = monitor_edge;
                }

                break;

            case GDK_GRAVITY_WEST:
                monitor_edge = geometry.x;

                if (monitor_edge < screen_edge)
                {
                    screen_edge = monitor_edge;
                }

                break;

            case GDK_GRAVITY_EAST:
                monitor_edge = GEO_RIGHT(geometry);

                if (monitor_edge > screen_edge)
                {
                    screen_edge = monitor_edge;
                }

                break;

            case GDK_GRAVITY_SOUTH:
                monitor_edge = GEO_BOTTOM(geometry);

                if (monitor_edge > screen_edge)
                {
                    screen_edge = monitor_edge;
                }

                break;

            default: break;
        }
    }

    // Retrieve the current window information
    //
    gint height_request = 0;
    gint width_request  = 0;

    gdk_monitor_get_geometry(monitor, &geometry);

    gtk_widget_get_size_request(
        GTK_WIDGET(wnd),
        &width_request,
        &height_request
    );

    // Set up struts and update window pos
    //
    struct X11Struts struts = { 0 };

    gint move_x = 0;
    gint move_y = 0;
    gint size_w = 0;
    gint size_h = 0;

    switch (priv->dock_side)
    {
        case GDK_GRAVITY_NORTH:
            struts.top =
                screen_edge + geometry.y + height_request;
            struts.top_start_x = geometry.x;
            struts.top_end_x   = GEO_RIGHT(geometry);

            move_x = geometry.x;
            move_y = geometry.y;
            size_w = geometry.width;
            size_h = height_request;
            break;

        case GDK_GRAVITY_WEST:
            struts.left =
                screen_edge + geometry.x + width_request;
            struts.left_start_y = geometry.y;
            struts.left_end_y   = GEO_BOTTOM(geometry);

            move_x = geometry.x;
            move_y = geometry.y;
            size_w = width_request;
            size_h = geometry.height;
            break;

        case GDK_GRAVITY_EAST:
            struts.right =
                screen_edge - GEO_RIGHT(geometry) + width_request;
            struts.right_start_y = geometry.y;
            struts.right_end_y   = GEO_BOTTOM(geometry);

            move_x = GEO_RIGHT(geometry) - width_request;
            move_y = geometry.y;
            size_w = width_request;
            size_h = geometry.height;
            break;

        case GDK_GRAVITY_SOUTH:
            struts.bottom =
                screen_edge - GEO_BOTTOM(geometry) + height_request;
            struts.bottom_start_x = geometry.x;
            struts.bottom_end_x   = GEO_RIGHT(geometry);

            move_x = geometry.x;
            move_y = GEO_BOTTOM(geometry) - height_request;
            size_w = geometry.width;
            size_h = height_request;
            break;

        default: break;
    }

    gtk_widget_set_size_request(
        GTK_WIDGET(wnd),
        size_w,
        size_h
    );
    gtk_window_move(
        GTK_WINDOW(wnd),
        move_x,
        move_y
    );

    gdk_property_change(
        gtk_widget_get_window(GTK_WIDGET(wnd)),
        net_wm_strut_partial_atom,
        cardinal_atom,
        32,
        GDK_PROP_MODE_REPLACE,
        (guchar*) &struts,
        sizeof (struct X11Struts) / sizeof (gulong)
    );

    // Update last size request so we avoid size-allocate firing a bunch
    //
    if (
        priv->dock_side == GDK_GRAVITY_NORTH ||
        priv->dock_side == GDK_GRAVITY_SOUTH
    )
    {
        priv->last_size = height_request;
    }
    else // EAST || WEST
    {
        priv->last_size = width_request;
    }
}

static void window_setup_wayland(
    WinTCDpaPanelWindow* wnd
)
{
    p_gtk_layer_init_for_window(GTK_WINDOW(wnd));
    p_gtk_layer_set_layer(GTK_WINDOW(wnd), GTK_LAYER_SHELL_LAYER_BOTTOM);
    p_gtk_layer_auto_exclusive_zone_enable(GTK_WINDOW(wnd));
}

static void window_setup_x11(
    WinTCDpaPanelWindow* wnd
)
{
    g_signal_connect(
        wnd,
        "realize",
        G_CALLBACK(on_window_realize_x11),
        NULL
    );
    g_signal_connect(
        wnd,
        "size-allocate",
        G_CALLBACK(on_window_size_allocate_x11),
        NULL
    );

    // React to monitor changes
    //
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(
            gdk_display_get_default()
        );

    g_signal_connect_object(
        monitor,
        "notify::geometry",
        G_CALLBACK(on_monitor_notify_geometry),
        wnd,
        G_CONNECT_DEFAULT
    );
}

//
// CALLBACKS
//
static void on_monitor_notify_geometry(
    WINTC_UNUSED(GObject* self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    update_x11_struts(WINTC_DPA_PANEL_WINDOW(user_data));
}

static void on_window_realize_x11(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    update_x11_struts(WINTC_DPA_PANEL_WINDOW(self));
}

static void on_window_size_allocate_x11(
    GtkWidget*    widget,
    GdkRectangle* allocation,
    WINTC_UNUSED(gpointer user_data)
)
{
    WinTCDpaPanelWindow* wnd = WINTC_DPA_PANEL_WINDOW(widget);
    WinTCDpaPanelWindowPrivate* priv =
        wintc_dpa_panel_window_get_instance_private(wnd);

    // No-op if the size didn't change
    //
    gint cmp;

    if (
        priv->dock_side == GDK_GRAVITY_NORTH ||
        priv->dock_side == GDK_GRAVITY_SOUTH
    )
    {
        cmp = allocation->height;
    }
    else // EAST || WEST
    {
        cmp = allocation->width;
    }

    if (priv->last_size == cmp)
    {
        return;
    }

    // Update
    //
    update_x11_struts(wnd);
}
