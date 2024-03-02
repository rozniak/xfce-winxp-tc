#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/winbrand.h>

#include "stripctl.h"

#define ONE_SECOND_IN_US  1000000

#define ANIM_RATE_SECONDS 2
#define ANIM_RATE_IN_US   (ANIM_RATE_SECONDS * ONE_SECOND_IN_US)

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCGinaStripClass
{
    GtkWidgetClass __parent__;    
};

struct _WinTCGinaStrip
{
    GtkWidget __parent__;

    // UI resources and state
    //
    GdkPixbuf*       pixbuf_strip;
    cairo_surface_t* surface_strip;

    gboolean is_animating;
    gint64   origin_time;
};

//
// FORWARD DECLARATIONS
//
static void wintc_gina_strip_finalize(
    GObject* gobject
);

static gboolean wintc_gina_strip_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_gina_strip_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_gina_strip_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_gina_strip_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_gina_strip_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);

static gboolean wintc_gina_strip_step(
    GtkWidget*     widget,
    GdkFrameClock* frame_clock,
    gpointer       user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCGinaStrip,
    wintc_gina_strip,
    GTK_TYPE_WIDGET
)

static void wintc_gina_strip_class_init(
    WinTCGinaStripClass* klass
)
{
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_gina_strip_finalize;

    widget_class->draw                           =
        wintc_gina_strip_draw;
    widget_class->get_preferred_height           =
        wintc_gina_strip_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        wintc_gina_strip_get_preferred_height_for_width;
    widget_class->get_preferred_width            =
        wintc_gina_strip_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        wintc_gina_strip_get_preferred_width_for_height;
}

static void wintc_gina_strip_init(
    WinTCGinaStrip* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);

    // Load assets
    //
    GError* error = NULL;

    self->pixbuf_strip =
        wintc_brand_get_brand_pixmap(
            WINTC_BRAND_PART_STRIP_ANIM,
            &error
        );

    // FIXME: Handle error

    self->surface_strip =
        gdk_cairo_surface_create_from_pixbuf(
            self->pixbuf_strip,
            1,
            NULL // FIXME: Error reporting
        );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_strip_finalize(
    GObject* gobject
)
{
    WinTCGinaStrip* strip = WINTC_GINA_STRIP(gobject);

    cairo_surface_destroy(strip->surface_strip);
    g_clear_object(&(strip->pixbuf_strip));

    (G_OBJECT_CLASS(wintc_gina_strip_parent_class))->finalize(gobject);
}

static gboolean wintc_gina_strip_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCGinaStrip* strip    = WINTC_GINA_STRIP(widget);
    gdouble         x_offset = 0.0f;

    // If animating, work out offset of strip based
    // on time
    //
    if (strip->is_animating)
    {
        gint64  current_time = g_get_monotonic_time();
        gint64  delta        = strip->origin_time - current_time;
        gint64  mod_time     = delta % ANIM_RATE_IN_US;
        gdouble within_pct   = (gdouble) mod_time / ANIM_RATE_IN_US;

        gint img_width =
            cairo_image_surface_get_width(
                strip->surface_strip
            );

        x_offset = (gint) (img_width * within_pct);
    }

    // Paint now
    //
    cairo_set_source_surface(
        cr,
        strip->surface_strip,
        x_offset,
        0.0f
    );
    cairo_pattern_set_extend(
        cairo_get_source(cr),
        CAIRO_EXTEND_REPEAT
    );

    cairo_paint(cr);

    return FALSE;
}

static void wintc_gina_strip_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
)
{
    gint            height;
    WinTCGinaStrip* strip = WINTC_GINA_STRIP(widget);

    height =
        cairo_image_surface_get_height(
            strip->surface_strip
        );

    *minimum_height = height;
    *natural_height = height;
}

static void wintc_gina_strip_get_preferred_height_for_width(
    GtkWidget* widget,
    WINTC_UNUSED(gint width),
    gint*      minimum_height,
    gint*      natural_height
)
{
    wintc_gina_strip_get_preferred_height(
        widget,
        minimum_height,
        natural_height
    );
}

static void wintc_gina_strip_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
)
{
    WinTCGinaStrip* strip = WINTC_GINA_STRIP(widget);
    gint            width;

    width =
        cairo_image_surface_get_width(
            strip->surface_strip
        );

    *minimum_width = width;
    *natural_width = width;
}

static void wintc_gina_strip_get_preferred_width_for_height(
    GtkWidget* widget,
    WINTC_UNUSED(gint height),
    gint*      minimum_width,
    gint*      natural_width
)
{
    wintc_gina_strip_get_preferred_width(
        widget,
        minimum_width,
        natural_width
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_strip_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            TYPE_WINTC_GINA_STRIP,
            NULL
        )
    );
}

void wintc_gina_strip_animate(
    WinTCGinaStrip* strip
)
{
    if (strip->is_animating)
    {
        return;
    }

    strip->is_animating = TRUE;
    strip->origin_time  = g_get_monotonic_time();

    gtk_widget_add_tick_callback(
        GTK_WIDGET(strip),
        (GtkTickCallback) wintc_gina_strip_step,
        NULL,
        NULL
    );
}

void wintc_gina_strip_stop_animating(
    WinTCGinaStrip* strip
)
{
    strip->is_animating = FALSE;
}

//
// CALLBACKS
//
static gboolean wintc_gina_strip_step(
    GtkWidget*     widget,
    WINTC_UNUSED(GdkFrameClock* frame_clock),
    WINTC_UNUSED(gpointer       user_data)
)
{
    WinTCGinaStrip* strip = WINTC_GINA_STRIP(widget);

    gtk_widget_queue_draw(widget);

    return strip->is_animating ?
        G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

