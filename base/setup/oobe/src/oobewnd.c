#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "oobewnd.h"

//
// FORWARD DECLARATIONS
//
static void cb_st_eos(
    GstBus*     bus,
    GstMessage* msg,
    gpointer    user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCOobeWindowClass
{
    WinTCDpaDesktopWindowClass __parent__;
};

struct _WinTCOobeWindow
{
    WinTCDpaDesktopWindow __parent__;

    // GStreamer
    //
    GstElement* gst_playbin;
    GtkWidget*  sink_intro;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCOobeWindow,
    wintc_oobe_window,
    WINTC_TYPE_DPA_DESKTOP_WINDOW
)

static void wintc_oobe_window_class_init(
    WINTC_UNUSED(WinTCOobeWindowClass* klass)
)
{
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/oobe/oobewnd.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static void wintc_oobe_window_init(
    WinTCOobeWindow* self
)
{
    wintc_widget_add_style_class(
        GTK_WIDGET(self),
        "wintc-oobe"
    );

    // Set up GStreamer playback for intro.wmv
    //
    GstElement* gst_videosink;
    GstElement* gst_gtkglsink;

    self->gst_playbin = gst_element_factory_make("playbin", "playbin");

    gst_videosink = gst_element_factory_make("glsinkbin", "glsinkbin");
    gst_gtkglsink = gst_element_factory_make("gtkglsink", "gtkglsink");

    if (gst_videosink && gst_gtkglsink)
    {
        g_object_set(
            gst_videosink,
            "sink",
            gst_gtkglsink,
            NULL
        );

        g_object_get(
            gst_gtkglsink,
            "widget",
            &(self->sink_intro),
            NULL
        );
    }
    else
    {
        gst_videosink =
            gst_element_factory_make("gtksink", "gtksink");

        g_object_get(
            gst_videosink,
            "widget",
            &(self->sink_intro),
            NULL
        );
    }

    if (!self->gst_playbin || !gst_videosink)
    {
        // FIXME: Handle this by skipping straight to the wizard
        //
        g_critical("%s", "oobe: couldn't play intro.wmv");
        return;
    }

    g_object_set(
        self->gst_playbin,
        "uri",        "file://" WINTC_ASSETS_DIR "/oobe/intro.wmv",
        "video-sink", gst_videosink,
        NULL
    );

    gtk_container_add(GTK_CONTAINER(self), self->sink_intro);

    GstBus* bus = gst_element_get_bus(self->gst_playbin);

    gst_bus_add_signal_watch(bus);

    g_signal_connect(
        bus,
        "message::eos",
        G_CALLBACK(cb_st_eos),
        self
    );

    gst_object_unref(bus);

    gst_element_set_state(self->gst_playbin, GST_STATE_PLAYING);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_oobe_window_new(void)
{
    GdkMonitor* monitor =
        gdk_display_get_primary_monitor(
            gdk_screen_get_display(
                gdk_screen_get_default()
            )
        );

    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_OOBE_WINDOW,
            "type",    GTK_WINDOW_TOPLEVEL,
            "monitor", monitor,
            NULL
        )
    );
}

//
// CALLBACKS
//
static void cb_st_eos(
    WINTC_UNUSED(GstBus*     bus),
    WINTC_UNUSED(GstMessage* msg),
    gpointer user_data
)
{
    // FIXME: Chain to wizard
    wintc_container_clear(GTK_CONTAINER(user_data), TRUE);
}
