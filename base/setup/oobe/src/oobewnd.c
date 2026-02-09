#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "oobewnd.h"

//
// FORWARD DECLARATIONS
//
static gboolean wintc_oobe_window_init_intro(
    WinTCOobeWindow* window
);

static void cb_st_eos(
    GstBus*     bus,
    GstMessage* msg,
    gpointer    user_data
);
static void on_gst_intro_decode_pad_added(
    GstElement* src,
    GstPad*     new_pad,
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

    // GStreamer (intro.wmv)
    //
    GstElement* gst_intro_pipeline;

    GstElement* gst_intro_decode;
    GstElement* gst_intro_convert;
    GstElement* gst_intro_sink;

    GstElement* gst_playbin;
    GtkWidget*  gtksink_intro;
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

    // Set up intro.wmv to play
    //
    if (wintc_oobe_window_init_intro(self))
    {
        GstBus* bus = gst_element_get_bus(self->gst_intro_pipeline);

        gst_bus_add_signal_watch(bus);

        g_signal_connect(
            bus,
            "message::eos",
            G_CALLBACK(cb_st_eos),
            self
        );

        gst_object_unref(bus);

        gtk_container_add(
            GTK_CONTAINER(self),
            self->gtksink_intro
        );

        gst_element_set_state(
            self->gst_intro_pipeline,
            GST_STATE_PLAYING
        );
    }
    else
    {
        // FIXME: Handle this by skipping straight to the wizard
        //
        g_critical("%s", "oobe: couldn't play intro.wmv");
        return;
    }
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
// PRIVATE FUNCTIONS
//
static gboolean wintc_oobe_window_init_intro(
    WinTCOobeWindow* wnd_oobe
)
{
    wnd_oobe->gst_intro_decode  = gst_element_factory_make(
                                    "uridecodebin",
                                    "src"
                                );
    wnd_oobe->gst_intro_convert = gst_element_factory_make(
                                    "videoconvert",
                                    "cvt"
                                );
    wnd_oobe->gst_intro_sink    = gst_element_factory_make(
                                    "gtksink",
                                    "sink"
                                );

    wnd_oobe->gst_intro_pipeline =
        gst_pipeline_new("intro_wmv");

    if (
        !(wnd_oobe->gst_intro_decode)   ||
        !(wnd_oobe->gst_intro_convert)  ||
        !(wnd_oobe->gst_intro_sink)     ||
        !(wnd_oobe->gst_intro_pipeline)
    )
    {
        WINTC_LOG_DEBUG("oobe: failed to create intro.wmv pipeline");
        return FALSE;
    }

    // Construct pipeline
    //
    gst_bin_add_many(
        GST_BIN(wnd_oobe->gst_intro_pipeline),
        wnd_oobe->gst_intro_decode,
        wnd_oobe->gst_intro_convert,
        wnd_oobe->gst_intro_sink,
        NULL
    );

    g_object_set(
        wnd_oobe->gst_intro_sink,
        "force-aspect-ratio", FALSE,
        NULL
    );

    if (
        !gst_element_link_many(
            wnd_oobe->gst_intro_convert,
            wnd_oobe->gst_intro_sink,
            NULL
        )
    )
    {
        WINTC_LOG_DEBUG("oobe: failed to link up intro.wmv elements");
        return FALSE;
    }

    // Get set up with intro.wmv
    //
    g_object_set(
        wnd_oobe->gst_intro_decode,
        "uri", "file://" WINTC_ASSETS_DIR "/oobe/intro.wmv",
        NULL
    );

    g_signal_connect(
        wnd_oobe->gst_intro_decode,
        "pad-added",
        G_CALLBACK(on_gst_intro_decode_pad_added),
        wnd_oobe
    );

    // Pull the GTK widget
    //
    g_object_get(
        wnd_oobe->gst_intro_sink,
        "widget", &(wnd_oobe->gtksink_intro),
        NULL
    );

    return TRUE;
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

static void on_gst_intro_decode_pad_added(
    WINTC_UNUSED(GstElement* src),
    GstPad*  new_pad,
    gpointer user_data
)
{
    WinTCOobeWindow* wnd_oobe = WINTC_OOBE_WINDOW(user_data);

    GstCaps*      new_pad_caps   = NULL;
    GstStructure* new_pad_struct = NULL;
    const gchar*  new_pad_mime   = NULL;

    // Use videoconvert as the sink
    //
    GstPad* sink_pad =
        gst_element_get_static_pad(wnd_oobe->gst_intro_convert, "sink");

    if (gst_pad_is_linked(sink_pad))
    {
        WINTC_LOG_DEBUG("oobe: intro.wmv new pad, but already linked");
        goto cleanup;
    }

    // Inspect the pad
    //
    new_pad_caps   = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_mime   = gst_structure_get_name(new_pad_struct);

    if (g_strcmp0(new_pad_mime, "video/x-raw") != 0)
    {
        WINTC_LOG_DEBUG(
            "oobe: intro.wmv skipping pad of type %s",
            new_pad_mime
        );
        goto cleanup;
    }

    GstPadLinkReturn ret =
        gst_pad_link(new_pad, sink_pad);

    if (GST_PAD_LINK_FAILED(ret))
    {
        WINTC_LOG_DEBUG("oobe: failed to link new pad for intro.wmv");
    }
    else
    {
        WINTC_LOG_DEBUG("oobe: link successful for intro.wmv");
    }

cleanup:
    if (new_pad_caps)
    {
        gst_caps_unref(new_pad_caps);
    }

    gst_object_unref(sink_pad);
}
