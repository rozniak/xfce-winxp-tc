#include <glib.h>
#include <pulse/pulseaudio.h>
#include <wintc/comgtk.h>

#include "../public/context.h"
#include "../public/output.h"
#include "output-priv.h"
#include "volcvt.h"

//
// GTK OOP BOILERPLATE
//
enum
{
    PROP_PA_CONTEXT = 1,
    PROP_PA_SINK_ID
};

enum
{
    SIGNAL_VOLUME_CHANGED = 0,
    SIGNAL_MUTED_CHANGED,
    N_SIGNALS
};

//
// STATIC DATA
//
static gint wintc_sndapi_output_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSndApiOutputClass
{
    GObjectClass __parent__;
};

struct _WinTCSndApiOutput
{
    GObject __parent__;

    pa_context* pulse_context;
    uint32_t    sink_id;
    gchar*      sink_name;

    gboolean muted;
    gdouble  volume;

    gboolean next_muted;
    gdouble  next_volume;
};

//
// FORWARD DECLARATIONS
//
static void wintc_sndapi_output_constructed(
    GObject* object
);
static void wintc_sndapi_output_finalize(
    GObject* object
);
static void wintc_sndapi_output_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sndapi_output_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_sndapi_output_internal_set_muted(
    WinTCSndApiOutput* output,
    gboolean           muted
);
static void wintc_sndapi_output_internal_set_volume(
    WinTCSndApiOutput* output,
    gdouble            volume
);

static void sndopt_pulse_sink_info_set_volume_cb(
    pa_context*         c,
    const pa_sink_info* i,
    int                 eol,
    void*               userdata
);
static void sndopt_pulse_success_set_mute_cb(
    pa_context* c,
    int         success,
    void*       userdata
);
static void sndopt_pulse_success_set_volume_cb(
    pa_context* c,
    int         success,
    void*       userdata
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCSndApiOutput,
    wintc_sndapi_output,
    G_TYPE_OBJECT
)

static void wintc_sndapi_output_class_init(
    WinTCSndApiOutputClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sndapi_output_constructed;
    object_class->finalize     = wintc_sndapi_output_finalize;
    object_class->get_property = wintc_sndapi_output_get_property;
    object_class->set_property = wintc_sndapi_output_set_property;

    wintc_sndapi_output_signals[SIGNAL_VOLUME_CHANGED] =
        g_signal_new(
            "volume-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_sndapi_output_signals[SIGNAL_MUTED_CHANGED] =
        g_signal_new(
            "muted-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );

    g_object_class_install_property(
        object_class,
        PROP_PA_CONTEXT,
        g_param_spec_pointer(
            "pa-context",
            "PaContext",
            "The PulseAudio connection context.",
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_PA_SINK_ID,
        g_param_spec_uint(
            "pa-sink-id",
            "PaSinkId",
            "The ID of the PulseAudio sink that the instance represents.",
            0,
            G_MAXUINT,
            0,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_sndapi_output_init(
    WINTC_UNUSED(WinTCSndApiOutput* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sndapi_output_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(
        wintc_sndapi_output_parent_class
    ))->constructed(object);
}

static void wintc_sndapi_output_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCSndApiOutput* output =
        WINTC_SNDAPI_OUTPUT(object);

    switch (prop_id)
    {
        case PROP_PA_SINK_ID:
            g_value_set_uint(value, output->sink_id);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_sndapi_output_finalize(
    GObject* object
)
{
    WinTCSndApiOutput* output = WINTC_SNDAPI_OUTPUT(object);

    g_free(output->sink_name);
}

static void wintc_sndapi_output_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCSndApiOutput* output =
        WINTC_SNDAPI_OUTPUT(object);

    switch (prop_id)
    {
        case PROP_PA_CONTEXT:
            output->pulse_context = g_value_get_pointer(value);
            break;

        case PROP_PA_SINK_ID:
            output->sink_id = g_value_get_uint(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
gdouble wintc_sndapi_output_get_volume(
    WinTCSndApiOutput* output
)
{
    return output->volume;
}

gboolean wintc_sndapi_output_is_muted(
    WinTCSndApiOutput* output
)
{
    return output->muted;
}

void wintc_sndapi_output_set_muted(
    WinTCSndApiOutput* output,
    gboolean           muted
)
{
    pa_operation* o;

    output->next_muted = muted;

    if (
        !(o =
            pa_context_set_sink_mute_by_index(
                output->pulse_context,
                output->sink_id,
                output->next_muted,
                sndopt_pulse_success_set_mute_cb,
                output
            )
        )
    )
    {
        g_error("%s", "Failed to set mute state on sink.");
        return;
    }

    pa_operation_unref(o);
}

void wintc_sndapi_output_set_volume(
    WinTCSndApiOutput* output,
    gdouble            new_volume
)
{
    pa_operation* o;

    output->next_volume = new_volume;

    if (
        !(o =
            pa_context_get_sink_info_by_index(
                output->pulse_context,
                output->sink_id,
                sndopt_pulse_sink_info_set_volume_cb,
                output
            )
        )
    )
    {
        g_error("%s", "Failed to set volume on sink.");
        return;
    }

    pa_operation_unref(o);
}

//
// INTERNAL FUNCTIONS
//
WinTCSndApiOutput* wintc_sndapi_output_new(
    pa_context* pulse_context,
    uint32_t    sink_id
)
{
    return WINTC_SNDAPI_OUTPUT(
        g_object_new(
            WINTC_TYPE_SNDAPI_OUTPUT,
            "pa-context", pulse_context,
            "pa-sink-id", sink_id,
            NULL
        )
    );
}

void wintc_sndapi_output_update_from_sink_info(
    WinTCSndApiOutput*  output,
    const pa_sink_info* i
)
{
    if (g_strcmp0(i->name, output->sink_name) != 0)
    {
        WINTC_LOG_DEBUG("Sink name changed to %s", i->name);

        g_free(output->sink_name);
        output->sink_name = g_strdup(i->name);
    }

    // Read highest volume
    //
    gdouble vol = 0.0f;

    for (uint8_t idx = 0; idx < i->volume.channels; idx++)
    {
        vol =
            MAX(
                vol,
                wintc_sndapi_cvt_pa_volume_to_percent(i->volume.values[idx])
            );
    }

    wintc_sndapi_output_internal_set_volume(
        output,
        vol
    );

    // Read muted
    //
    wintc_sndapi_output_internal_set_muted(
        output,
        !!(i->mute)
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_sndapi_output_internal_set_muted(
    WinTCSndApiOutput* output,
    gboolean           muted
)
{
    WINTC_LOG_DEBUG("SNDAPI: Int mute change to %d", muted);

    output->muted = muted;

    g_signal_emit(
        output,
        wintc_sndapi_output_signals[SIGNAL_MUTED_CHANGED],
        0
    );
}

static void wintc_sndapi_output_internal_set_volume(
    WinTCSndApiOutput* output,
    gdouble            volume
)
{
    WINTC_LOG_DEBUG("SNDAPI: Int vol change to %f", volume);

    output->volume = volume;

    g_signal_emit(
        output,
        wintc_sndapi_output_signals[SIGNAL_VOLUME_CHANGED],
        0
    );
}

//
// CALLBACKS
//
static void sndopt_pulse_sink_info_set_volume_cb(
    pa_context*         c,
    const pa_sink_info* i,
    WINTC_UNUSED(int eol),
    void*               userdata
)
{
    WinTCSndApiOutput* output = WINTC_SNDAPI_OUTPUT(userdata);

    if (i == NULL)
    {
        return;
    }

    pa_volume_t desired_volume = wintc_sndapi_cvt_percent_to_pa_volume(
                                     output->next_volume
                                 );
    pa_volume_t highest_volume = pa_cvolume_max(&(i->volume));
    pa_cvolume  new_cvolume    = i->volume;
    gboolean    okay           = FALSE;

    if (desired_volume == highest_volume)
    {
        return;
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
        g_warning("%s", "Failed to inc/dec volume.");
    }

    // Actually apply the new volume
    //
    pa_operation* o;

    if (
        !(o =
            pa_context_set_sink_volume_by_index(
                c,
                output->sink_id,
                &new_cvolume,
                sndopt_pulse_success_set_volume_cb,
                output
            )
        )
    )
    {
        g_error("%s", "Failed to set cvolume on sink.");
        return;
    }

    pa_operation_unref(o);
}

static void sndopt_pulse_success_set_mute_cb(
    WINTC_UNUSED(pa_context* c),
    int   success,
    void* userdata
)
{
    WinTCSndApiOutput* output = WINTC_SNDAPI_OUTPUT(userdata);

    if (!success)
    {
        return;
    }

    wintc_sndapi_output_internal_set_muted(
        output,
        output->next_muted
    );
}

static void sndopt_pulse_success_set_volume_cb(
    WINTC_UNUSED(pa_context* c),
    int   success,
    void* userdata
)
{
    WinTCSndApiOutput* output = WINTC_SNDAPI_OUTPUT(userdata);

    if (!success)
    {
        return;
    }

    wintc_sndapi_output_internal_set_volume(
        output,
        output->next_volume
    );
}
