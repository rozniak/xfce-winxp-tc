#include <glib.h>
#include <gtk/gtk.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <wintc/comgtk.h>

#include "application.h"
#include "window.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCPaTestWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCPaTestWindow
{
    GtkApplicationWindow __parent__;

    // UI widgets
    //
    GtkWidget* label_text;
    GtkWidget* label_volume;
    GtkWidget* scale_volume;
    GtkWidget* toggle_mute;

    // PulseAudio stuff
    //
    pa_context*       pulse_context;
    pa_glib_mainloop* pulse_mainloop;

    gchar* pulse_default_sink;

    gboolean sink_muted;
    gdouble  sink_volume;
    gboolean updating_from_read;
};

//
// FORWARD DECLARATIONS
//
static void wintc_patest_window_finalize(
    GObject* gobject
);

static void wintc_patest_window_set_sink_muted(
    WinTCPaTestWindow* wnd,
    gboolean           muted,
    gboolean           push
);
static void wintc_patest_window_set_sink_volume(
    WinTCPaTestWindow* wnd,
    gdouble            volume_pct,
    gboolean           push
);
static void wintc_patest_window_update_from_pa(
    WinTCPaTestWindow* wnd
);

static gdouble pa_volume_to_percent(
    pa_volume_t volume
);
static pa_volume_t percent_to_pa_volume(
    gdouble pct
);

static void on_scale_volume_value_changed(
    GtkRange* self,
    gpointer  user_data
);
static void on_toggle_mute_toggled(
    GtkToggleButton* self,
    gpointer         user_data
);

static void wintc_patest_pulse_server_info_cb(
    pa_context*           c,
    const pa_server_info* i,
    void*                 userdata
);
static void wintc_patest_pulse_sink_info_for_read_cb(
    pa_context*         c,
    const pa_sink_info* i,
    int                 eol,
    void*               userdata
);
static void wintc_patest_pulse_sink_info_for_write_cb(
    pa_context*         c,
    const pa_sink_info* i,
    int                 eol,
    void*               userdata
);
static void wintc_patest_pulse_state_cb(
    pa_context* c,
    void*       userdata
);
static void wintc_patest_pulse_subscribe_cb(
    pa_context*                  c,
    pa_subscription_event_type_t t,
    uint32_t                     idx,
    void*                        userdata
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCPaTestWindow,
    wintc_patest_window,
    GTK_TYPE_APPLICATION_WINDOW
)

static void wintc_patest_window_class_init(
    WinTCPaTestWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = wintc_patest_window_finalize;
}

static void wintc_patest_window_init(
    WinTCPaTestWindow* self
)
{
    // Set up window
    //
    gtk_window_set_default_size(
        GTK_WINDOW(self),
        320,
        200
    );

    // Set up UI
    //
    GtkWidget* box_container = gtk_box_new(GTK_ORIENTATION_VERTICAL,   0);
    GtkWidget* box_mute      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    self->label_text   = gtk_label_new("Hello");
    self->label_volume = gtk_label_new("?%");
    self->scale_volume = gtk_scale_new_with_range(
                             GTK_ORIENTATION_HORIZONTAL,
                             0.0,
                             1.0,
                             0.05
                         );
    self->toggle_mute  = gtk_toggle_button_new_with_label("Mute");

    gtk_scale_set_draw_value(GTK_SCALE(self->scale_volume), FALSE);

    g_signal_connect(
        self->scale_volume,
        "value-changed",
        G_CALLBACK(on_scale_volume_value_changed),
        self
    );
    g_signal_connect(
        self->toggle_mute,
        "toggled",
        G_CALLBACK(on_toggle_mute_toggled),
        self
    );

    gtk_box_pack_end(
        GTK_BOX(box_mute),
        self->toggle_mute,
        FALSE,
        FALSE,
        0
    );

    gtk_box_pack_start(
        GTK_BOX(box_container),
        self->label_text,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(box_container),
        self->scale_volume,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(box_container),
        self->label_volume,
        FALSE,
        FALSE,
        0
    );
    gtk_box_pack_start(
        GTK_BOX(box_container),
        box_mute,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        box_container
    );

    // PA initial state
    //
    self->sink_muted         = FALSE;
    self->sink_volume        = 0.0f;
    self->updating_from_read = FALSE;

    // PulseAudio stuff
    //
    gint err = 0;

    self->pulse_mainloop = pa_glib_mainloop_new(NULL);
    self->pulse_context  = pa_context_new(
                               pa_glib_mainloop_get_api(self->pulse_mainloop),
                               "wintc-patest"
                           );

    pa_context_set_state_callback(
        self->pulse_context,
        wintc_patest_pulse_state_cb,
        self
    );

    err =
        pa_context_connect(
            self->pulse_context,
            NULL,
            PA_CONTEXT_NOFAIL,
            NULL
        );

    if (err < 0)
    {
        g_warning("PA connect failure: %s", pa_strerror(err));
    }
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_patest_window_finalize(
    GObject* gobject
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(gobject);

    pa_glib_mainloop_free(wnd->pulse_mainloop);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_patest_window_new(
    WinTCPaTestApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_PATEST_WINDOW,
            "application", GTK_APPLICATION(app),
            "title",       "PulseAudio Test Program",
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_patest_window_set_sink_muted(
    WinTCPaTestWindow* wnd,
    gboolean           muted,
    gboolean           push
)
{
    wnd->sink_muted = muted;

    g_message(
        "Set sink muted, new mute status is %d",
        muted
    );

    wnd->updating_from_read = TRUE;
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(wnd->toggle_mute),
        muted
    );
    wnd->updating_from_read = FALSE;

    if (push)
    {
        pa_context_set_sink_mute_by_name(
            wnd->pulse_context,
            wnd->pulse_default_sink,
            wnd->sink_muted,
            NULL,
            NULL
        );
    }
}

static void wintc_patest_window_set_sink_volume(
    WinTCPaTestWindow* wnd,
    gdouble            volume_pct,
    gboolean           push
)
{
    wnd->sink_volume = volume_pct;

    g_message(
        "Set sink volume, new volume is: %f",
        volume_pct
    );

    wnd->updating_from_read = TRUE;
    gtk_range_set_value(GTK_RANGE(wnd->scale_volume), volume_pct);
    wnd->updating_from_read = FALSE;

    if (push)
    {
        // Have to use the sink info callback in order to retrieve
        // volume information in order to adjust it
        //
        pa_context_get_sink_info_by_name(
            wnd->pulse_context,
            wnd->pulse_default_sink,
            wintc_patest_pulse_sink_info_for_write_cb,
            wnd
        );
    }
}

static void wintc_patest_window_update_from_pa(
    WinTCPaTestWindow* wnd
)
{
    pa_context_get_server_info(
        wnd->pulse_context,
        wintc_patest_pulse_server_info_cb,
        wnd
    );

    pa_context_get_sink_info_by_name(
        wnd->pulse_context,
        wnd->pulse_default_sink,
        wintc_patest_pulse_sink_info_for_read_cb,
        wnd
    );
}

static gdouble pa_volume_to_percent(
    pa_volume_t volume
)
{
    gdouble adjusted_vol;
    gdouble range;

    adjusted_vol = (gdouble) (volume - PA_VOLUME_MUTED);
    range        = (gdouble) (PA_VOLUME_NORM - PA_VOLUME_MUTED);

    return adjusted_vol / range;
}

static pa_volume_t percent_to_pa_volume(
    gdouble pct
)
{
    gdouble     range;
    pa_volume_t scaled;

    range = (gdouble) (PA_VOLUME_NORM - PA_VOLUME_MUTED);
    scaled = (pa_volume_t) (pct * range);

    return scaled + PA_VOLUME_MUTED;
}

//
// CALLBACKS
//
static void on_scale_volume_value_changed(
    GtkRange* self,
    gpointer  user_data
)
{
    gchar*             str_vol = NULL;
    WinTCPaTestWindow* wnd     = WINTC_PATEST_WINDOW(user_data);

    // Update UI
    //
    str_vol = g_strdup_printf("Volume: %f%%", gtk_range_get_value(self));

    gtk_label_set_text(GTK_LABEL(wnd->label_volume), str_vol);
    g_message("Scale value changed %s", str_vol);

    g_free(str_vol);

    // No need to do anything if this signal was raised by a PulseAudio event
    // rather than the user
    //
    if (wnd->updating_from_read)
    {
        return;
    }

    // User was responsible for changing the value, push the change to
    // PulseAudio
    //
    wintc_patest_window_set_sink_volume(
        wnd,
        gtk_range_get_value(self),
        TRUE
    );
}

static void on_toggle_mute_toggled(
    GtkToggleButton* self,
    gpointer         user_data
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(user_data);

    if (wnd->updating_from_read)
    {
        return;
    }

    wintc_patest_window_set_sink_muted(
        wnd,
        gtk_toggle_button_get_active(self),
        TRUE
    );
}

static void wintc_patest_pulse_server_info_cb(
    WINTC_UNUSED(pa_context* c),
    const pa_server_info* i,
    void*                 userdata
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(userdata);

    if (g_strcmp0(i->default_sink_name, wnd->pulse_default_sink) == 0)
    {
        return;
    }

    g_free(wnd->pulse_default_sink);
    wnd->pulse_default_sink = g_strdup(i->default_sink_name);
}

static void wintc_patest_pulse_sink_info_for_read_cb(
    WINTC_UNUSED(pa_context* c),
    const pa_sink_info* i,
    WINTC_UNUSED(int eol),
    void*               userdata
)
{
    gdouble            vol = 0.0;
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(userdata);

    if (i == NULL)
    {
        return;
    }

    // Read highest volume
    //
    for (uint8_t idx = 0; idx < i->volume.channels; idx++)
    {
        vol = MAX(vol, pa_volume_to_percent(i->volume.values[idx]));
    }

    g_message("Read in - sink says volume is %f", vol);

    wintc_patest_window_set_sink_volume(wnd, vol, FALSE);

    // Set muted
    //
    wintc_patest_window_set_sink_muted(wnd, !!(i->mute), FALSE);
}

static void wintc_patest_pulse_sink_info_for_write_cb(
    pa_context*         c,
    const pa_sink_info* i,
    WINTC_UNUSED(int eol),
    void*               userdata
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(userdata);

    if (i == NULL)
    {
        return;
    }

    pa_volume_t desired_volume = percent_to_pa_volume(wnd->sink_volume);
    pa_volume_t highest_volume = pa_cvolume_max(&(i->volume));
    pa_cvolume  new_cvolume    = i->volume;
    gboolean    okay           = FALSE;

    if (desired_volume == highest_volume)
    {
        return; // No-op
    }

    if (desired_volume > highest_volume)
    {
        okay =
            pa_cvolume_inc_clamp(
                &new_cvolume,
                desired_volume - highest_volume,
                PA_VOLUME_NORM
            ) != NULL;
    }
    else
    {
        okay =
            pa_cvolume_dec(
                &new_cvolume,
                highest_volume - desired_volume
            ) != NULL;
    }

    if (!okay)
    {
        // Failed to change volume... for some reason
        //
        g_warning("%s", "Volume inc/dec returned NULL");
    }

    g_message(
        "Changing old volume %f to %f",
        pa_volume_to_percent(highest_volume),
        pa_volume_to_percent(pa_cvolume_max(&new_cvolume))
    );

    pa_context_set_sink_volume_by_name(
        c,
        wnd->pulse_default_sink,
        &new_cvolume,
        NULL,
        NULL
    );
}

static void wintc_patest_pulse_state_cb(
    pa_context* c,
    void*       userdata
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(userdata);

    gboolean enable = FALSE;
    gchar*   msg    = "Nothing";

    switch (pa_context_get_state(c))
    {
        case PA_CONTEXT_UNCONNECTED:
            msg = "PACTX Not Connected.";
            break;

        case PA_CONTEXT_CONNECTING:
            msg = "PACTX connecting...";
            break;

        case PA_CONTEXT_AUTHORIZING:
            msg = "PACTX authorizing...";
            break;

        case PA_CONTEXT_SETTING_NAME:
            msg = "PACTX setting name";
            break;

        case PA_CONTEXT_READY:
            enable = TRUE;
            msg    = "PACTX Ready.";

            pa_context_subscribe(
                c,
                PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER,
                NULL,
                NULL
            );
            pa_context_set_subscribe_callback(
                c,
                wintc_patest_pulse_subscribe_cb,
                wnd
            );

            wintc_patest_window_update_from_pa(wnd);

            break;

        case PA_CONTEXT_FAILED:
            msg = "PACTX Failed";
            break;

        case PA_CONTEXT_TERMINATED:
            msg = "PACTX Terminated!";
            break;
    }

    g_message("%s", msg);
    gtk_label_set_text(GTK_LABEL(wnd->label_text), msg);
    gtk_widget_set_sensitive(wnd->scale_volume, enable);
    gtk_widget_set_sensitive(wnd->toggle_mute,  enable);
}

static void wintc_patest_pulse_subscribe_cb(
    WINTC_UNUSED(pa_context* c),
    pa_subscription_event_type_t t,
    WINTC_UNUSED(uint32_t idx),
    void*                        userdata
)
{
    WinTCPaTestWindow* wnd = WINTC_PATEST_WINDOW(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
        case PA_SUBSCRIPTION_EVENT_SERVER:
        case PA_SUBSCRIPTION_EVENT_SINK:
            wintc_patest_window_update_from_pa(wnd);
            break;

        default:
            g_warning("%s", "Unknown event received.");
            break;
    }
}
