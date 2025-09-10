#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "icon.h"

#define TRAY_ICON_SIZE 16

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_ICON_NAME,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_notif_area_icon_dispose(
    GObject* object
);
static void wintc_notif_area_icon_finalize(
    GObject* object
);
static void wintc_notif_area_icon_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_notif_area_icon_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_notif_area_icon_draw(
    GtkWidget* widget,
    cairo_t*   cr
);
static void wintc_notif_area_icon_get_preferred_height(
    GtkWidget* widget,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_notif_area_icon_get_preferred_height_for_width(
    GtkWidget* widget,
    gint       width,
    gint*      minimum_height,
    gint*      natural_height
);
static void wintc_notif_area_icon_get_preferred_width(
    GtkWidget* widget,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_notif_area_icon_get_preferred_width_for_height(
    GtkWidget* widget,
    gint       height,
    gint*      minimum_width,
    gint*      natural_width
);
static void wintc_notif_area_icon_map(
    GtkWidget* widget
);
static void wintc_notif_area_icon_realize(
    GtkWidget* widget
);
static void wintc_notif_area_icon_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
);
static void wintc_notif_area_icon_unmap(
    GtkWidget* widget
);
static void wintc_notif_area_icon_unrealize(
    GtkWidget* widget
);

//
// STATIC DATA
//
static GParamSpec* wintc_notif_area_icon_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotifAreaIcon
{
    GtkWidget __parent__;

    GdkWindow*       hwnd;
    gchar*           icon_name;
    GdkPixbuf*       pixbuf_icon;
    cairo_surface_t* surface_icon;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotifAreaIcon,
    wintc_notif_area_icon,
    GTK_TYPE_WIDGET
)

static void wintc_notif_area_icon_class_init(
    WinTCNotifAreaIconClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose      = wintc_notif_area_icon_dispose;
    object_class->finalize     = wintc_notif_area_icon_finalize;
    object_class->get_property = wintc_notif_area_icon_get_property;
    object_class->set_property = wintc_notif_area_icon_set_property;

    widget_class->draw          = wintc_notif_area_icon_draw;
    widget_class->map           = wintc_notif_area_icon_map;
    widget_class->realize       = wintc_notif_area_icon_realize;
    widget_class->size_allocate = wintc_notif_area_icon_size_allocate;
    widget_class->unmap         = wintc_notif_area_icon_unmap;
    widget_class->unrealize     = wintc_notif_area_icon_unrealize;

    widget_class->get_preferred_height           =
        wintc_notif_area_icon_get_preferred_height;
    widget_class->get_preferred_height_for_width =
        wintc_notif_area_icon_get_preferred_height_for_width;
    widget_class->get_preferred_width            =
        wintc_notif_area_icon_get_preferred_width;
    widget_class->get_preferred_width_for_height =
        wintc_notif_area_icon_get_preferred_width_for_height;

    wintc_notif_area_icon_properties[PROP_ICON_NAME] =
        g_param_spec_string(
            "icon-name",
            "IconName",
            "The XDG icon name to use.",
            "image-missing",
            G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_notif_area_icon_properties
    );
}

static void wintc_notif_area_icon_init(
    WinTCNotifAreaIcon* self
)
{
    gtk_widget_set_has_window(GTK_WIDGET(self), FALSE);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notif_area_icon_dispose(
    GObject* object
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(object);

    if (notif_icon->pixbuf_icon)
    {
        cairo_surface_destroy(notif_icon->surface_icon);
        g_clear_object(&(notif_icon->pixbuf_icon));
    }

    (G_OBJECT_CLASS(wintc_notif_area_icon_parent_class))
        ->dispose(object);
}

static void wintc_notif_area_icon_finalize(
    GObject* object
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(object);

    g_free(notif_icon->icon_name);

    (G_OBJECT_CLASS(wintc_notif_area_icon_parent_class))
        ->finalize(object);
}

static void wintc_notif_area_icon_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            g_value_set_string(value, notif_icon->icon_name);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_notif_area_icon_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            wintc_notif_area_icon_set_icon_name(
                notif_icon,
                g_value_get_string(value)
            );
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean wintc_notif_area_icon_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    if (!(notif_icon->surface_icon))
    {
        return FALSE;
    }

    // We draw the icon centered vertically
    //
    gint height = gtk_widget_get_allocated_height(widget);

    cairo_save(cr);

    cairo_translate(
        cr,
        0.0f,
        (double) ((height / 2) - (TRAY_ICON_SIZE / 2))
    );

    cairo_set_source_surface(
        cr,
        notif_icon->surface_icon,
        0.0f,
        0.0f
    );
    cairo_pattern_set_extend(
        cairo_get_source(cr),
        CAIRO_EXTEND_NONE
    );

    cairo_paint(cr);

    cairo_restore(cr);

    return FALSE;
}

static void wintc_notif_area_icon_get_preferred_height(
    WINTC_UNUSED(GtkWidget* widget),
    gint* minimum_height,
    gint* natural_height
)
{
    *minimum_height = TRAY_ICON_SIZE;
    *natural_height = TRAY_ICON_SIZE;
}

static void wintc_notif_area_icon_get_preferred_height_for_width(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gint       width),
    gint* minimum_height,
    gint* natural_height
)
{
    *minimum_height = TRAY_ICON_SIZE;
    *natural_height = TRAY_ICON_SIZE;
}

static void wintc_notif_area_icon_get_preferred_width(
    WINTC_UNUSED(GtkWidget* widget),
    gint* minimum_width,
    gint* natural_width
)
{
    *minimum_width = TRAY_ICON_SIZE;
    *natural_width = TRAY_ICON_SIZE;
}

static void wintc_notif_area_icon_get_preferred_width_for_height(
    WINTC_UNUSED(GtkWidget* widget),
    WINTC_UNUSED(gint       height),
    gint* minimum_width,
    gint* natural_width
)
{
    *minimum_width = TRAY_ICON_SIZE;
    *natural_width = TRAY_ICON_SIZE;
}

static void wintc_notif_area_icon_map(
    GtkWidget* widget
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    if (notif_icon->hwnd)
    {
        gdk_window_show(notif_icon->hwnd);
    }

    (GTK_WIDGET_CLASS(wintc_notif_area_icon_parent_class))
        ->map(widget);
}

static void wintc_notif_area_icon_realize(
    GtkWidget* widget
)
{
    (GTK_WIDGET_CLASS(wintc_notif_area_icon_parent_class))
        ->realize(widget);

    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    GtkAllocation allocation;
    GdkWindowAttr attribs;

    gtk_widget_get_allocation(widget, &allocation);

    attribs.x           = allocation.x;
    attribs.y           = allocation.y;
    attribs.width       = allocation.width;
    attribs.height      = allocation.height;
    attribs.event_mask  = GDK_BUTTON_PRESS_MASK   |
                          GDK_BUTTON_RELEASE_MASK |
                          GDK_POINTER_MOTION_MASK;
    attribs.wclass      = GDK_INPUT_ONLY;
    attribs.window_type = GDK_WINDOW_CHILD;

    notif_icon->hwnd =
        gdk_window_new(
            gtk_widget_get_parent_window(widget),
            &attribs,
            GDK_WA_X | GDK_WA_Y
        );

    gtk_widget_register_window(widget, notif_icon->hwnd);
}

static void wintc_notif_area_icon_size_allocate(
    GtkWidget*     widget,
    GtkAllocation* allocation
)
{
    (GTK_WIDGET_CLASS(wintc_notif_area_icon_parent_class))
        ->size_allocate(widget, allocation);

    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    if (gtk_widget_get_realized(widget) && notif_icon->hwnd)
    {
        gdk_window_move_resize(
            notif_icon->hwnd,
            allocation->x,
            allocation->y,
            allocation->width,
            allocation->height
        );
    }
}

static void wintc_notif_area_icon_unmap(
    GtkWidget* widget
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    if (notif_icon->hwnd)
    {
        gdk_window_hide(notif_icon->hwnd);
    }

    (GTK_WIDGET_CLASS(wintc_notif_area_icon_parent_class))
        ->unmap(widget);
}

static void wintc_notif_area_icon_unrealize(
    GtkWidget* widget
)
{
    WinTCNotifAreaIcon* notif_icon = WINTC_NOTIF_AREA_ICON(widget);

    if (notif_icon->hwnd)
    {
        gtk_widget_unregister_window(widget, notif_icon->hwnd);
        gdk_window_destroy(notif_icon->hwnd);

        notif_icon->hwnd = NULL;
    }

    (GTK_WIDGET_CLASS(wintc_notif_area_icon_parent_class))
        ->unrealize(widget);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_notif_area_icon_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTIF_AREA_ICON,
            NULL
        )
    );
}

const gchar* wintc_notif_area_icon_get_icon_name(
    WinTCNotifAreaIcon* notif_icon
)
{
    return notif_icon->icon_name;
}

void wintc_notif_area_icon_set_icon_name(
    WinTCNotifAreaIcon* notif_icon,
    const gchar*        icon_name
)
{
    // Bin old icon
    //
    if (notif_icon->icon_name)
    {
        g_free(notif_icon->icon_name);
    }

    if (notif_icon->pixbuf_icon)
    {
        cairo_surface_destroy(notif_icon->surface_icon);
        g_clear_object(&(notif_icon->pixbuf_icon));
    }

    // Set new icon
    //
    notif_icon->icon_name = g_strdup(icon_name);

    notif_icon->pixbuf_icon =
        gtk_icon_theme_load_icon(
            gtk_icon_theme_get_default(),
            notif_icon->icon_name,
            16,
            GTK_ICON_LOOKUP_FORCE_SIZE,
            NULL
        );

    if (notif_icon->pixbuf_icon)
    {
        notif_icon->surface_icon =
            gdk_cairo_surface_create_from_pixbuf(
                notif_icon->pixbuf_icon,
                1,
                NULL
            );
    }

    // Finish up
    //
    gtk_widget_queue_draw(GTK_WIDGET(notif_icon));

    g_object_notify_by_pspec(
        G_OBJECT(notif_icon),
        wintc_notif_area_icon_properties[PROP_ICON_NAME]
    );
}
