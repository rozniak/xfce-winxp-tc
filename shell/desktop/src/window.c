#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>
#include <wintc/syscfg.h>

#include "application.h"
#include "settings.h"
#include "window.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SETTINGS,
    PROP_SHEXT_HOST,
    N_PROPERTIES
};

//
// FORWARD DECLARATIONS
//
static void wintc_desktop_window_constructed(
    GObject* object
);
static void wintc_desktop_window_dispose(
    GObject* object
);
static void wintc_desktop_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static gboolean wintc_desktop_window_draw(
    GtkWidget* widget,
    cairo_t*   cr
);

static void wintc_desktop_window_update_wallpaper(
    WinTCDesktopWindow* wnd
);

static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);

static void on_settings_notify_pixbuf_wallpaper(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_settings_notify_wallpaper_style(
    GObject*    self,
    GParamSpec* pspec,
    gpointer    user_data
);

//
// STATIC DATA
//
static GParamSpec* wintc_desktop_window_properties[N_PROPERTIES] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCDesktopWindow
{
    WinTCDpaDesktopWindow __parent__;

    // UI
    //
    GtkWidget*                iconview_browser;
    WinTCShIconViewBehaviour* behaviour_icons;

    // Shell stuff
    //
    WinTCShBrowser*       browser;
    WinTCShFolderOptions* fldr_opts;
    WinTCShextHost*       shext_host;

    // Drawing-related
    //
    GdkPixbuf*       pixbuf_wallpaper;
    cairo_surface_t* surface_wallpaper;

    // State
    //
    WinTCDesktopSettings* settings;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCDesktopWindow,
    wintc_desktop_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_desktop_window_class_init(
    WinTCDesktopWindowClass* klass
)
{
    GObjectClass*   object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

    object_class->constructed  = wintc_desktop_window_constructed;
    object_class->dispose      = wintc_desktop_window_dispose;
    object_class->set_property = wintc_desktop_window_set_property;

    widget_class->draw = wintc_desktop_window_draw;

    wintc_desktop_window_properties[PROP_SETTINGS] =
        g_param_spec_object(
            "settings",
            "Settings",
            "The shared desktop settings",
            WINTC_TYPE_DESKTOP_SETTINGS,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );
    wintc_desktop_window_properties[PROP_SHEXT_HOST] =
        g_param_spec_object(
            "shext-host",
            "ShextHost",
            "The shell extension host object to use.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_desktop_window_properties
    );
}

static void wintc_desktop_window_init(
    WinTCDesktopWindow* self
)
{
    self->iconview_browser = gtk_icon_view_new();

    gtk_container_add(
        GTK_CONTAINER(self),
        self->iconview_browser
    );

    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );

    wintc_widget_add_style_class(
        GTK_WIDGET(self),
        "wintc-desktop"
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_desktop_window_constructed(
    GObject* object
)
{
    WinTCDesktopWindow* wnd = WINTC_DESKTOP_WINDOW(object);

    // If we have a shext host, then we can set up the browser
    //
    if (wnd->shext_host)
    {
        wnd->fldr_opts = wintc_sh_folder_options_new();

        wintc_sh_folder_options_set_browse_in_same_window(
            wnd->fldr_opts,
            FALSE
        );

        wnd->browser =
            wintc_sh_browser_new(
                wnd->shext_host,
                wnd->fldr_opts
            );

        wintc_sh_browser_set_behaviour_flags(
            wnd->browser,
            WINTC_SH_BROWSER_BEHAVIOUR_DONT_USE_SELF_EXE
        );

        wnd->behaviour_icons =
            wintc_sh_icon_view_behaviour_new(
                GTK_ICON_VIEW(wnd->iconview_browser),
                wnd->browser
            );

        // Navigate to desktop
        //
        WinTCShextPathInfo path_info = { 0 };

        path_info.base_path =
            g_strdup(
                wintc_sh_get_place_path(WINTC_SH_PLACE_DESKTOP)
            );

        wintc_sh_browser_set_location(
            wnd->browser,
            &path_info,
            NULL
        );

        wintc_shext_path_info_free_data(&path_info);
    }

    g_signal_connect(
        wnd->settings,
        "notify::pixbuf-wallpaper",
        G_CALLBACK(on_settings_notify_pixbuf_wallpaper),
        wnd
    );
    g_signal_connect(
        wnd->settings,
        "notify::wallpaper-style",
        G_CALLBACK(on_settings_notify_wallpaper_style),
        wnd
    );

    (G_OBJECT_CLASS(wintc_desktop_window_parent_class))->constructed(object);
}

static void wintc_desktop_window_dispose(
    GObject* object
)
{
    WinTCDesktopWindow* wnd = WINTC_DESKTOP_WINDOW(object);

    g_clear_object(&(wnd->behaviour_icons));
    g_clear_object(&(wnd->browser));
    g_clear_object(&(wnd->fldr_opts));
    g_clear_object(&(wnd->shext_host));

    if (wnd->surface_wallpaper)
    {
        cairo_surface_destroy(g_steal_pointer(&(wnd->surface_wallpaper)));
        g_clear_object(&(wnd->pixbuf_wallpaper));
    }

    (G_OBJECT_CLASS(wintc_desktop_window_parent_class))->dispose(object);
}

static void wintc_desktop_window_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCDesktopWindow* wnd = WINTC_DESKTOP_WINDOW(object);

    switch (prop_id)
    {
        case PROP_SETTINGS:
            wnd->settings = g_value_get_object(value);
            break;

        case PROP_SHEXT_HOST:
            wnd->shext_host = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean wintc_desktop_window_draw(
    GtkWidget* widget,
    cairo_t*   cr
)
{
    WinTCDesktopWindow* wnd = WINTC_DESKTOP_WINDOW(widget);

    gint wnd_w = gtk_widget_get_allocated_width(widget);
    gint wnd_h = gtk_widget_get_allocated_height(widget);

    // FIXME: Just drawing default desktop background colour atm
    //
    //cairo_set_source_rgb(cr, 0.0f, 0.298f, 0.596f);
    //cairo_paint(cr);

    // FIXME: Just drawing the style background colour atm
    //
    GtkStyleContext* ctx = gtk_widget_get_style_context(widget);

    gtk_render_background(
        ctx,
        cr,
        0.0f,
        0.0f,
        (gdouble) wnd_w,
        (gdouble) wnd_h
    );

    // FIXME: Billy basic drawing of the wallpaper, if present
    //
    gint wallpaper_style = 0;

    if (wnd->surface_wallpaper)
    {
        gint wallpaper_w = gdk_pixbuf_get_width(wnd->pixbuf_wallpaper);
        gint wallpaper_h = gdk_pixbuf_get_height(wnd->pixbuf_wallpaper);

        cairo_save(cr);

        g_object_get(
            wnd->settings,
            "wallpaper-style", &wallpaper_style,
            NULL
        );

        switch (wallpaper_style)
        {
            case WINTC_WALLPAPER_STYLE_CENTER:
                cairo_translate(
                    cr,
                    ((gdouble) wnd_w / 2) - ((gdouble) wallpaper_w / 2),
                    ((gdouble) wnd_h / 2) - ((gdouble) wallpaper_h / 2)
                );
                break;

            case WINTC_WALLPAPER_STYLE_STRETCH:
                cairo_scale(
                    cr,
                    (gdouble) wnd_w / wallpaper_w,
                    (gdouble) wnd_h / wallpaper_h
                );
                break;
        }

        cairo_set_source_surface(cr, wnd->surface_wallpaper, 0.0f, 0.0f);

        if (wallpaper_style == WINTC_WALLPAPER_STYLE_TILED)
        {
            cairo_pattern_set_extend(
                cairo_get_source(cr),
                CAIRO_EXTEND_REPEAT
            );
        }

        cairo_paint(cr);

        cairo_restore(cr);
    }

    // Rough watermark drawing
    //
    // NOTE: We assume that we're the primary monitor if we have a shext host
    //       passed in -- this is quite cheeky to do, until we properly deal
    //       with primary monitor business on Waheyland
    //
    if (wnd->shext_host)
    {
        static gchar* s_tag = NULL;

        cairo_text_extents_t extents;
        gdouble              y;

        gint height = gtk_widget_get_allocated_height(widget);
        gint width  = gtk_widget_get_allocated_width(widget);

        if (!s_tag)
        {
            s_tag =
                g_strdup_printf(
                    "For testing purposes only. Build %s",
                    wintc_build_query(WINTC_VER_TAG)
                );
        }

        // Default to white, IDK if Windows changes this...
        //
        cairo_set_source_rgb(cr, 1.0f, 1.0f, 1.0f);

        // Handle build string first...
        //
        cairo_select_font_face(
            cr,
            "sans-serif",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_NORMAL
        );

        cairo_text_extents(cr, s_tag, &extents);

        cairo_move_to(
            cr,
            width - extents.width - 10,
            height - extents.height - 34 // FIXME: Hardcoded taskband height
        );

        cairo_show_text(cr, s_tag);

        // ...and the product name
        // FIXME: Hard coded temp name in here until we have a name
        //
        cairo_select_font_face(
            cr,
            "sans-serif",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD
        );

        cairo_text_extents(cr, "Windows 'Total Conversion'", &extents);

        cairo_get_current_point(cr, NULL, &y);

        cairo_move_to(
            cr,
            width - extents.width - 10,
            y - extents.height - 4
        );

        cairo_show_text(cr, "Windows 'Total Conversion'");
    }

    // Draw the icon view
    //
    gtk_widget_draw(wnd->iconview_browser, cr);

//    (GTK_WIDGET_CLASS(wintc_desktop_window_parent_class))
//        ->draw(widget, cr);

    return FALSE;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_desktop_window_new(
    WinTCDesktopApplication* app,
    GdkMonitor*              monitor,
    WinTCDesktopSettings*    settings,
    WinTCShextHost*          shext_host
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_DESKTOP_WINDOW,
            "application", GTK_APPLICATION(app),
            "type",        GTK_WINDOW_TOPLEVEL,
            "decorated",   TRUE,
            "monitor",     monitor,
            "resizable",   FALSE,
            "settings",    settings,
            "shext-host",  shext_host,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_desktop_window_update_wallpaper(
    WinTCDesktopWindow* wnd
)
{
    WINTC_LOG_DEBUG("%s", "desktop: window updating wallpaper");

    if (wnd->surface_wallpaper)
    {
        cairo_surface_destroy(g_steal_pointer(&(wnd->surface_wallpaper)));
        g_clear_object(&(wnd->pixbuf_wallpaper));
    }

    g_object_get(
        wnd->settings,
        "pixbuf-wallpaper", &(wnd->pixbuf_wallpaper),
        NULL
    );

    if (wnd->pixbuf_wallpaper)
    {
        wnd->surface_wallpaper =
            gdk_cairo_surface_create_from_pixbuf(
                wnd->pixbuf_wallpaper,
                1,
                NULL
            );

        gtk_widget_queue_draw(GTK_WIDGET(wnd));
    }
}

//
// CALLBACKS
//
static gboolean on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny* event),
    WINTC_UNUSED(gpointer user_data)
)
{
    wintc_desktop_window_update_wallpaper(
        WINTC_DESKTOP_WINDOW(self)
    );

    return FALSE;
}

static void on_settings_notify_pixbuf_wallpaper(
    WINTC_UNUSED(GObject* self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    wintc_desktop_window_update_wallpaper(
        WINTC_DESKTOP_WINDOW(user_data)
    );
}

static void on_settings_notify_wallpaper_style(
    WINTC_UNUSED(GObject* self),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    gtk_widget_queue_draw(GTK_WIDGET(user_data));
}
