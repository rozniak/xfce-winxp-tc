#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <math.h>
#include <wintc/comgtk.h>

#include "monitor.h"

//
// STATIC CONSTANTS
//
static const gint S_PREVIEW_IMAGE_OFFSET_X = 16;
static const gint S_PREVIEW_IMAGE_OFFSET_Y = 17;

static const gint S_PREVIEW_IMAGE_WIDTH  = 152;
static const gint S_PREVIEW_IMAGE_HEIGHT = 112;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCDeskMonitorClass
{
    GtkWidgetClass __parent__;

    GdkPixbuf*       pixbuf_monitor;
    cairo_surface_t* surface_monitor;

    gint pixbuf_monitor_height;
    gint pixbuf_monitor_width;
};

struct _WinTCDeskMonitor
{
    GtkWidget __parent__;

    // UI
    //
    GdkPixbuf*       pixbuf_preview;
    cairo_surface_t* surface_preview;
};

//
// FORWARD DECLARATIONS
//
static void wintc_desk_monitor_dispose(
    GObject* object
);

static gboolean wintc_desk_monitor_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_desk_monitor_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_desk_monitor_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_desk_monitor_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_desk_monitor_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCDeskMonitor,
    wintc_desk_monitor,
    GTK_TYPE_WIDGET
)

static void wintc_desk_monitor_class_init(
    WinTCDeskMonitorClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_desk_monitor_dispose;

    widget_class->draw                           =
        wintc_desk_monitor_draw;
    widget_class->get_preferred_height           =
        wintc_desk_monitor_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        wintc_desk_monitor_get_preferred_height_for_width;
    widget_class->get_preferred_width            =
        wintc_desk_monitor_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        wintc_desk_monitor_get_preferred_width_for_height;

    // Load shared monitor graphic resource
    //
    GError* error = NULL;

    klass->pixbuf_monitor =
        gdk_pixbuf_new_from_resource(
            "/uk/oddmatics/wintc/cpl-desk/monitor.png",
            &error
        );

    wintc_log_error_and_clear(&error);

    if (klass->pixbuf_monitor)
    {
        klass->surface_monitor =
            gdk_cairo_surface_create_from_pixbuf(
                klass->pixbuf_monitor,
                1,
                NULL
            );

        klass->pixbuf_monitor_height =
            gdk_pixbuf_get_height(
                klass->pixbuf_monitor
            );
        klass->pixbuf_monitor_width  =
            gdk_pixbuf_get_width(
                klass->pixbuf_monitor
            );
    }
}

static void wintc_desk_monitor_init(
    WinTCDeskMonitor* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_desk_monitor_dispose(
    GObject* object
)
{
    WinTCDeskMonitor* monitor = WINTC_DESK_MONITOR(object);

    g_clear_object(&(monitor->pixbuf_preview));
    cairo_surface_destroy(g_steal_pointer(&monitor->surface_preview));

    (G_OBJECT_CLASS(wintc_desk_monitor_parent_class))->dispose(object);
}

static gboolean wintc_desk_monitor_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCDeskMonitorClass* klass   = WINTC_DESK_MONITOR_GET_CLASS(widget);
    WinTCDeskMonitor*      monitor = WINTC_DESK_MONITOR(widget);

    // monitor size
    gint monitor_height = klass->pixbuf_monitor_height;
    gint monitor_width  = klass->pixbuf_monitor_width;

    // my size
    gint my_height = gtk_widget_get_allocated_height(widget);
    gint my_width  = gtk_widget_get_allocated_width(widget);

    // drawing origin
    gdouble target_x = floor((my_width  / 2.0f) - (monitor_width  / 2.0f));
    gdouble target_y = floor((my_height / 2.0f) - (monitor_height / 2.0f));

    cairo_translate(
        cr,
        target_x,
        target_y
    );
    cairo_set_source_surface(
        cr,
        klass->surface_monitor,
        0.0f,
        0.0f
    );
    cairo_pattern_set_extend(
        cairo_get_source(cr),
        CAIRO_EXTEND_NONE
    );
    cairo_paint(cr);

    if (monitor->surface_preview)
    {
        cairo_translate(
            cr,
            S_PREVIEW_IMAGE_OFFSET_X,
            S_PREVIEW_IMAGE_OFFSET_Y
        );
        cairo_set_source_surface(
            cr,
            monitor->surface_preview,
            0.0f,
            0.0f
        );
        cairo_pattern_set_extend(
            cairo_get_source(cr),
            CAIRO_EXTEND_NONE
        );
        cairo_paint(cr);
    }

    return FALSE;
}

static void wintc_desk_monitor_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
)
{
    WinTCDeskMonitorClass* klass = WINTC_DESK_MONITOR_GET_CLASS(widget);

    *minimum_height = klass->pixbuf_monitor_height;
    *natural_height = klass->pixbuf_monitor_height;
}

static void wintc_desk_monitor_get_preferred_height_for_width(
    GtkWidget* widget,
    WINTC_UNUSED(gint width),
    gint*      minimum_height,
    gint*      natural_height
)
{
    wintc_desk_monitor_get_preferred_height(
        widget,
        minimum_height,
        natural_height
    );
}

static void wintc_desk_monitor_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
)
{
    WinTCDeskMonitorClass* klass = WINTC_DESK_MONITOR_GET_CLASS(widget);

    *minimum_width = klass->pixbuf_monitor_width;
    *natural_width = klass->pixbuf_monitor_width;
}

static void wintc_desk_monitor_get_preferred_width_for_height(
    GtkWidget* widget,
    WINTC_UNUSED(gint height),
    gint*      minimum_width,
    gint*      natural_width
)
{
    wintc_desk_monitor_get_preferred_width(
        widget,
        minimum_width,
        natural_width
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_desk_monitor_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_DESK_MONITOR,
            NULL
        )
    );
}

void wintc_desk_monitor_set_pixbuf(
    WinTCDeskMonitor* monitor,
    const GdkPixbuf*  pixbuf
)
{
    g_clear_object(&(monitor->pixbuf_preview));
    cairo_surface_destroy(g_steal_pointer(&monitor->surface_preview));

    monitor->pixbuf_preview =
        gdk_pixbuf_scale_simple(
            pixbuf,
            S_PREVIEW_IMAGE_WIDTH,
            S_PREVIEW_IMAGE_HEIGHT,
            GDK_INTERP_NEAREST
        );
    monitor->surface_preview =
        gdk_cairo_surface_create_from_pixbuf(
            monitor->pixbuf_preview,
            1,
            NULL
        );
}
