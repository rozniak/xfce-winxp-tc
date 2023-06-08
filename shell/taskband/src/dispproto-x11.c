#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "dispproto.h"
#include "dispproto-x11.h"
#include "taskband.h"

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
static void x11_anchor_taskband_to_bottom(
    GtkWindow* taskband
);

static void on_taskband_realized(
    GtkWidget* self,
    gpointer   user_data
);

//
// PUBLIC FUNCTIONS
//
gboolean init_x11_protocol_impl(void)
{
    anchor_taskband_to_bottom = &x11_anchor_taskband_to_bottom;

    return TRUE;
}

//
// PRIVATE FUNCTIONS
//
static void x11_anchor_taskband_to_bottom(
    GtkWindow* taskband
)
{
    g_signal_connect(
        taskband,
        "realize",
        G_CALLBACK(on_taskband_realized),
        NULL
    );
}

//
// CALLBACKS
//
static void on_taskband_realized(
    GtkWidget* self,
    WINTC_UNUSED(gpointer user_data)
)
{
    GdkAtom      cardinal_atom;
    GdkDisplay*  display       = gdk_display_get_default();
    GdkRectangle geometry;
    GdkMonitor*  monitor       = NULL;
    int          monitor_count = gdk_display_get_n_monitors(display);
    GdkAtom      net_wm_strut_partial_atom;
    int          screen_bottom = 0;

    cardinal_atom             =
        gdk_atom_intern_static_string("CARDINAL");
    net_wm_strut_partial_atom =
        gdk_atom_intern_static_string("_NET_WM_STRUT_PARTIAL");

    struct X11Struts struts = { 0 };

    for (int i = 0; i < monitor_count; i++)
    {
        int monitor_bottom = 0;
        GdkMonitor* monitor_i = gdk_display_get_monitor(display, i);

        if (monitor == NULL || gdk_monitor_is_primary(monitor_i))
        {
            monitor = monitor_i;
        }

        // Update screen bottom
        //
        gdk_monitor_get_geometry(monitor_i, &geometry);

        monitor_bottom = geometry.y + geometry.height;

        if (monitor_bottom > screen_bottom)
        {
            screen_bottom = monitor_bottom;
        }
    }

    gdk_monitor_get_geometry(monitor, &geometry);

    gtk_window_set_default_size(
        GTK_WINDOW(self),
        geometry.width,
        TASKBAND_ROW_HEIGHT
    );
    gtk_window_move(
        GTK_WINDOW(self),
        geometry.x,
        geometry.y + geometry.height - TASKBAND_ROW_HEIGHT
    );

    struts.bottom =
        screen_bottom - (geometry.y + geometry.height) + TASKBAND_ROW_HEIGHT;
    struts.bottom_start_x = geometry.x;
    struts.bottom_end_x = geometry.x + geometry.width;

    gdk_property_change(
        gtk_widget_get_window(self),
        net_wm_strut_partial_atom,
        cardinal_atom,
        32,
        GDK_PROP_MODE_REPLACE,
        (guchar*) &struts,
        sizeof (struct X11Struts) / sizeof (gulong)
    );
}
