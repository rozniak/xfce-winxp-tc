#include <glib.h>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "oobeuser.h"
#include "oobewnd.h"

//
// PRIVATE ENUMS
//
enum
{
    PAGE_WELCOME,
    PAGE_APPLY_CUSTOMISATIONS,
    PAGE_FINISH,
    N_PAGES
};

//
// FORWARD DECLARATIONS
//
static void wintc_oobe_window_go_to_page(
    WinTCOobeWindow* wnd_oobe,
    guint            page,
    gboolean         push_history
);
static gboolean wintc_oobe_window_start_intro(
    WinTCOobeWindow* wnd_oobe
);
static gboolean wintc_oobe_window_start_title(
    WinTCOobeWindow* wnd_oobe
);
static void wintc_oobe_window_set_action_enabled(
    WinTCOobeWindow* wnd_oobe,
    const gchar*     action_name,
    gboolean         enabled
);
static void wintc_oobe_window_start_wizard(
    WinTCOobeWindow* wnd_oobe
);

static void action_back(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_next(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);
static void action_skip(
    GSimpleAction* action,
    GVariant*      parameter,
    gpointer       user_data
);

static void cb_gst_intro_message(
    GstBus*     bus,
    GstMessage* msg,
    gpointer    user_data
);
static void on_gst_intro_decode_pad_added(
    GstElement* src,
    GstPad*     new_pad,
    gpointer    user_data
);
static void on_gst_title_decode_pad_added(
    GstElement* src,
    GstPad*     new_pad,
    gpointer    user_data
);

//
// STATIC DATA
//
static const GActionEntry S_ACTIONS[] = {
    {
        .name           = "back",
        .activate       = action_back,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "next",
        .activate       = action_next,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "skip",
        .activate       = action_skip,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

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

    GtkWidget*  gtksink_intro;

    // GStreamer (title.wma)
    //
    GstElement* gst_title_pipeline;

    GstElement* gst_title_decode;
    GstElement* gst_title_convert;
    GstElement* gst_title_resample;
    GstElement* gst_title_sink;

    // Wizard UI
    //
    GtkWidget* box_wizard;
    GtkWidget* stack_pages;

    GSList*    list_pages;

    GSList* list_history;
    guint   idx_current_page;
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

    // Insert actions
    //
    GSimpleActionGroup* action_group = g_simple_action_group_new();

    g_action_map_add_action_entries(
        G_ACTION_MAP(action_group),
        S_ACTIONS,
        G_N_ELEMENTS(S_ACTIONS),
        self
    );

    gtk_widget_insert_action_group(
        GTK_WIDGET(self),
        "win",
        G_ACTION_GROUP(action_group)
    );

    g_object_unref(action_group);

    // Create the wizard UI
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource("/uk/oddmatics/wintc/oobe/oobewnd.ui");

    wintc_builder_get_objects(
        builder,
        "main-box",    &(self->box_wizard),
        "stack-pages", &(self->stack_pages),
        NULL
    );

    g_object_ref(self->box_wizard);

    g_clear_object(&builder);

    // Add the wizard pages
    //
    for (guint i = 0; i < N_PAGES; i++)
    {
        gchar* resource_name =
            g_strdup_printf("/uk/oddmatics/wintc/oobe/oobepg%u.ui", i);

        builder =
            gtk_builder_new_from_resource(resource_name);

        self->list_pages =
            g_slist_prepend(
                self->list_pages,
                gtk_builder_get_object(builder, "page")
            );

        gtk_stack_add_named(
            GTK_STACK(self->stack_pages),
            GTK_WIDGET(self->list_pages->data),
            resource_name
        );

        g_free(resource_name);
        g_clear_object(&builder);
    }

    self->list_pages = g_slist_reverse(self->list_pages);

    // Set up intro.wmv to play
    //
    if (!wintc_oobe_window_start_intro(self))
    {
        g_warning("%s", "oobe: couldn't play intro.wmv");

        wintc_oobe_window_start_wizard(self);
    }

    // Set up title.wma to play
    //
    if (!wintc_oobe_window_start_title(self))
    {
        g_warning("%s", "oobe: couldn't play title.wma");
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
static void wintc_oobe_window_go_to_page(
    WinTCOobeWindow* wnd_oobe,
    guint            page,
    gboolean         push_history
)
{
    gboolean can_back = TRUE;
    gboolean can_skip = FALSE;
    gboolean can_next = TRUE;

    // Push current page first
    //
    if (push_history)
    {
        wnd_oobe->list_history =
            g_slist_prepend(
                wnd_oobe->list_history,
                GUINT_TO_POINTER(wnd_oobe->idx_current_page)
            );
    }

    // Nav to next page
    //
    wnd_oobe->idx_current_page = page;

    gtk_stack_set_visible_child(
        GTK_STACK(wnd_oobe->stack_pages),
        GTK_WIDGET(
            g_slist_nth_data(wnd_oobe->list_pages, page)
        )
    );

    // Determine action availability
    //
    switch (wnd_oobe->idx_current_page)
    {
        case PAGE_FINISH:
            can_back = FALSE;
            break;

        default: break;
    }

    // Update actions
    //
    wintc_oobe_window_set_action_enabled(
        wnd_oobe,
        "back",
        can_back && !!(wnd_oobe->list_history)
    );
    wintc_oobe_window_set_action_enabled(
        wnd_oobe,
        "skip",
        can_skip
    );
    wintc_oobe_window_set_action_enabled(
        wnd_oobe,
        "next",
        can_next
    );
}

static gboolean wintc_oobe_window_start_intro(
    WinTCOobeWindow* wnd_oobe
)
{
    wnd_oobe->gst_intro_decode  = gst_element_factory_make(
                                    "uridecodebin",
                                    "intro_src"
                                );
    wnd_oobe->gst_intro_convert = gst_element_factory_make(
                                    "videoconvert",
                                    "intro_cvt"
                                );
    wnd_oobe->gst_intro_sink    = gst_element_factory_make(
                                    "gtksink",
                                    "intro_sink"
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

    // Link up to receive messages on the bus and add the widget
    //
    GstBus* bus = gst_element_get_bus(wnd_oobe->gst_intro_pipeline);

    gst_bus_add_signal_watch(bus);

    g_signal_connect(
        bus,
        "message",
        G_CALLBACK(cb_gst_intro_message),
        wnd_oobe
    );

    gst_object_unref(bus);

    gtk_container_add(
        GTK_CONTAINER(wnd_oobe),
        wnd_oobe->gtksink_intro
    );

    return TRUE;
}

static gboolean wintc_oobe_window_start_title(
    WinTCOobeWindow* wnd_oobe
)
{
    wnd_oobe->gst_title_decode   = gst_element_factory_make(
                                       "uridecodebin",
                                       "title_src"
                                   );
    wnd_oobe->gst_title_convert  = gst_element_factory_make(
                                       "audioconvert",
                                       "title_cvt"
                                   );
    wnd_oobe->gst_title_resample = gst_element_factory_make(
                                       "audioresample",
                                       "title_res"
                                   );
    wnd_oobe->gst_title_sink     = gst_element_factory_make(
                                       "alsasink",
                                       "title_sink"
                                   );

    wnd_oobe->gst_title_pipeline =
        gst_pipeline_new("title_wma");

    if (
        !(wnd_oobe->gst_title_decode)   ||
        !(wnd_oobe->gst_title_convert)  ||
        !(wnd_oobe->gst_title_resample) ||
        !(wnd_oobe->gst_title_sink)     ||
        !(wnd_oobe->gst_title_pipeline)
    )
    {
        WINTC_LOG_DEBUG("oobe: failed to create title.wma pipeline");
        return FALSE;
    }

    // Construct pipeline
    //
    gst_bin_add_many(
        GST_BIN(wnd_oobe->gst_title_pipeline),
        wnd_oobe->gst_title_decode,
        wnd_oobe->gst_title_convert,
        wnd_oobe->gst_title_resample,
        wnd_oobe->gst_title_sink,
        NULL
    );

    if (
        !gst_element_link_many(
            wnd_oobe->gst_title_convert,
            wnd_oobe->gst_title_resample,
            wnd_oobe->gst_title_sink,
            NULL
        )
    )
    {
        WINTC_LOG_DEBUG("oobe: failed to link up title.wma elements");
        return FALSE;
    }

    // Get set up with intro.wmv
    //
    g_object_set(
        wnd_oobe->gst_title_decode,
        "uri", "file://" WINTC_ASSETS_DIR "/oobe/title.wma",
        NULL
    );

    g_signal_connect(
        wnd_oobe->gst_title_decode,
        "pad-added",
        G_CALLBACK(on_gst_title_decode_pad_added),
        wnd_oobe
    );

    return TRUE;
}

static void wintc_oobe_window_set_action_enabled(
    WinTCOobeWindow* wnd_oobe,
    const gchar*     action_name,
    gboolean         enabled
)
{
    GAction*      action;
    GActionGroup* action_group;

    action_group =
        gtk_widget_get_action_group(
            GTK_WIDGET(wnd_oobe),
            "win"
        );

    action =
        g_action_map_lookup_action(
            G_ACTION_MAP(action_group),
            action_name      
        );

    g_simple_action_set_enabled(
        G_SIMPLE_ACTION(action),
        enabled
    );
}

static void wintc_oobe_window_start_wizard(
    WinTCOobeWindow* wnd_oobe
)
{
    wintc_container_clear(
        GTK_CONTAINER(wnd_oobe),
        FALSE
    );

    if (wnd_oobe->gtksink_intro)
    {
        gtk_widget_destroy(wnd_oobe->gtksink_intro);
        wnd_oobe->gtksink_intro = NULL;
    }

    gtk_container_add(
        GTK_CONTAINER(wnd_oobe),
        wnd_oobe->box_wizard
    );

    wintc_oobe_window_go_to_page(
        wnd_oobe,
        PAGE_WELCOME,
        FALSE
    );
}

//
// CALLBACKS
//
static void action_back(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCOobeWindow* wnd_oobe = WINTC_OOBE_WINDOW(user_data);

    if (!(wnd_oobe->list_history))
    {
        return;
    }

    guint page =
        GPOINTER_TO_UINT(wnd_oobe->list_history->data);

    wnd_oobe->list_history =
        g_slist_delete_link(
            wnd_oobe->list_history,
            wnd_oobe->list_history
        );

    wintc_oobe_window_go_to_page(
        wnd_oobe,
        page,
        FALSE
    );
}

static void action_next(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    gpointer user_data
)
{
    WinTCOobeWindow* wnd_oobe = WINTC_OOBE_WINDOW(user_data);

    GError* error = NULL;

    switch (wnd_oobe->idx_current_page)
    {
        case PAGE_APPLY_CUSTOMISATIONS:
            if (!wintc_oobe_user_apply_all(&error))
            {
                wintc_display_error_and_clear(
                    &error,
                    GTK_WINDOW(wnd_oobe)
                );
                return;
            }

            break;

        case PAGE_FINISH:
            gtk_window_close(GTK_WINDOW(wnd_oobe));
            return;

        default: break;
    }

    wintc_oobe_window_go_to_page(
        wnd_oobe,
        wnd_oobe->idx_current_page + 1,
        TRUE
    );
}

static void action_skip(
    WINTC_UNUSED(GSimpleAction* action),
    WINTC_UNUSED(GVariant*      parameter),
    WINTC_UNUSED(gpointer user_data)
)
{
    // FIXME: Implement when needed
}

static void cb_gst_intro_message(
    WINTC_UNUSED(GstBus*     bus),
    GstMessage* msg,
    gpointer    user_data
)
{
    WinTCOobeWindow* wnd_oobe = WINTC_OOBE_WINDOW(user_data);

    if (
        msg->type == GST_MESSAGE_EOS ||
        (
            msg->type == GST_MESSAGE_ELEMENT &&
            gst_is_missing_plugin_message(msg)
        )
    )
    {
        wintc_oobe_window_start_wizard(wnd_oobe);
    }
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

        // Begin playback!
        //
        gst_element_set_state(
            wnd_oobe->gst_intro_pipeline,
            GST_STATE_PLAYING
        );
    }

cleanup:
    if (new_pad_caps)
    {
        gst_caps_unref(new_pad_caps);
    }

    gst_object_unref(sink_pad);
}

static void on_gst_title_decode_pad_added(
    WINTC_UNUSED(GstElement* src),
    GstPad*  new_pad,
    gpointer user_data
)
{
    WinTCOobeWindow* wnd_oobe = WINTC_OOBE_WINDOW(user_data);

    GstCaps*      new_pad_caps   = NULL;
    GstStructure* new_pad_struct = NULL;
    const gchar*  new_pad_mime   = NULL;

    // Use audioconvert as the sink
    //
    GstPad* sink_pad =
        gst_element_get_static_pad(wnd_oobe->gst_title_convert, "sink");

    if (gst_pad_is_linked(sink_pad))
    {
        WINTC_LOG_DEBUG("oobe: title.wma new pad, but already linked");
        goto cleanup;
    }

    // Inspect the pad
    //
    new_pad_caps   = gst_pad_get_current_caps(new_pad);
    new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
    new_pad_mime   = gst_structure_get_name(new_pad_struct);

    if (g_strcmp0(new_pad_mime, "audio/x-raw") != 0)
    {
        WINTC_LOG_DEBUG(
            "oobe: title.wma skipping pad of type %s",
            new_pad_mime
        );
        goto cleanup;
    }

    GstPadLinkReturn ret =
        gst_pad_link(new_pad, sink_pad);

    if (GST_PAD_LINK_FAILED(ret))
    {
        WINTC_LOG_DEBUG("oobe: failed to link new pad for title.wma");
    }
    else
    {
        WINTC_LOG_DEBUG("oobe: link successful for title.wma");

        // Begin playback!
        // 
        gst_element_set_state(
            wnd_oobe->gst_title_pipeline,
            GST_STATE_PLAYING
        );
    }

cleanup:
    if (new_pad_caps)
    {
        gst_caps_unref(new_pad_caps);
    }

    gst_object_unref(sink_pad);
}
