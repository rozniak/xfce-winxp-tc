#include <glib.h>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <wintc/comgtk.h>

#include "../public/context.h"
#include "output-priv.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_CONNECTED_CHANGED = 0,
    SIGNAL_DEFAULT_OUTPUT_CHANGED,
    N_SIGNALS
};

//
// STATIC DATA
//
static gint wintc_sndapi_context_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCSndApiContextClass
{
    GObjectClass __parent__;
};

struct _WinTCSndApiContext
{
    GObject __parent__;

    // PulseAudio stuff
    //
    pa_context*       pulse_context;
    pa_glib_mainloop* pulse_mainloop;

    gboolean           sink_default_changed;
    gchar*             sink_default_name;
    WinTCSndApiOutput* sink_default;
};

//
// FORWARD DECLARATIONS
//
static void wintc_sndapi_context_dispose(
    GObject* object
);
static void wintc_sndapi_context_finalize(
    GObject* object
);

static void wintc_sndapi_context_emit_connected_changed(
    WinTCSndApiContext* ctx
);
static void wintc_sndapi_context_update_from_pa(
    WinTCSndApiContext* ctx
);

static void sndctx_pulse_server_info_cb(
    pa_context*           c,
    const pa_server_info* i,
    void*                 userdata
);
static void sndctx_pulse_sink_info_update_default_cb(
    pa_context*         c,
    const pa_sink_info* i,
    int                 eol,
    void*               userdata
);
static void sndctx_pulse_state_cb(
    pa_context* c,
    void*       userdata
);
static void sndctx_pulse_subscribe_cb(
    pa_context*                  c,
    pa_subscription_event_type_t t,
    uint32_t                     idx,
    void*                        userdata
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCSndApiContext,
    wintc_sndapi_context,
    G_TYPE_OBJECT
)

static void wintc_sndapi_context_class_init(
    WinTCSndApiContextClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose  = wintc_sndapi_context_dispose;
    object_class->finalize = wintc_sndapi_context_finalize;

    wintc_sndapi_context_signals[SIGNAL_CONNECTED_CHANGED] =
        g_signal_new(
            "connected-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__BOOLEAN,
            G_TYPE_NONE,
            1,
            G_TYPE_BOOLEAN
        );
    wintc_sndapi_context_signals[SIGNAL_DEFAULT_OUTPUT_CHANGED] =
        g_signal_new(
            "default-output-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_sndapi_context_init(
    WinTCSndApiContext* self
)
{
    // Set up PulseAudio stuff for connection later
    //
    self->pulse_mainloop = pa_glib_mainloop_new(NULL);
    self->pulse_context  = pa_context_new(
                               pa_glib_mainloop_get_api(self->pulse_mainloop),
                               "wintc-sndapi" // FIXME: Should be client name?
                           );

    pa_context_set_state_callback(
        self->pulse_context,
        sndctx_pulse_state_cb,
        self
    );

    // Initial stuff
    //
    self->sink_default_changed = FALSE;
    self->sink_default_name    = NULL;
    self->sink_default         = NULL;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sndapi_context_dispose(
    GObject* object
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(object);

    g_clear_object(&(ctx->sink_default));

    (G_OBJECT_CLASS(wintc_sndapi_context_parent_class))->dispose(object);
}

static void wintc_sndapi_context_finalize(
    GObject* object
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(object);

    g_free(ctx->sink_default_name);

    (G_OBJECT_CLASS(wintc_sndapi_context_parent_class))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCSndApiContext* wintc_sndapi_context_new(void)
{
    return WINTC_SNDAPI_CONTEXT(
        g_object_new(
            WINTC_TYPE_SNDAPI_CONTEXT,
            NULL
        )
    );
}

void wintc_sndapi_context_connect(
    WinTCSndApiContext* ctx
)
{
    if (wintc_sndapi_context_is_connected(ctx))
    {
        return;
    }

    gint err =
        pa_context_connect(
            ctx->pulse_context,
            NULL,
            PA_CONTEXT_NOFAIL,
            NULL
        );

    if (err < 0)
    {
        g_warning("sndapi: PA connect failure: %s", pa_strerror(err));
    }
}

WinTCSndApiOutput* wintc_sndapi_context_get_default_output(
    WinTCSndApiContext* ctx
)
{
    return ctx->sink_default;
}

gboolean wintc_sndapi_context_is_connected(
    WinTCSndApiContext* ctx
)
{
    return pa_context_get_state(ctx->pulse_context) == PA_CONTEXT_READY;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_sndapi_context_emit_connected_changed(
    WinTCSndApiContext* ctx
)
{
    g_signal_emit(
        ctx,
        wintc_sndapi_context_signals[SIGNAL_CONNECTED_CHANGED],
        0,
        wintc_sndapi_context_is_connected(ctx)
    );
}

static void wintc_sndapi_context_update_from_pa(
    WinTCSndApiContext* ctx
)
{
    pa_operation* o;

    if (
        !(o =
            pa_context_get_server_info(
                ctx->pulse_context,
                sndctx_pulse_server_info_cb,
                ctx
            )
        )
    )
    {
        g_error("%s", "Failed to get server info from PA.");
        return;
    }

    pa_operation_unref(o);
}

//
// CALLBACKS
//
static void sndctx_pulse_server_info_cb(
    pa_context*           c,
    const pa_server_info* i,
    void*                 userdata
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(userdata);
    pa_operation*       o;

    if (g_strcmp0(i->default_sink_name, ctx->sink_default_name) != 0)
    {
        ctx->sink_default_changed = TRUE;

        g_clear_object(&(ctx->sink_default));
        g_free(ctx->sink_default_name);
        ctx->sink_default_name = g_strdup(i->default_sink_name);
    }

    if (ctx->sink_default_name == NULL)
    {
        return;
    }

    if (
        !(o =
            pa_context_get_sink_info_by_name(
                c,
                ctx->sink_default_name,
                sndctx_pulse_sink_info_update_default_cb,
                ctx
            )
        )
    )
    {
        g_error("%s", "Failed to get default sink info from PA.");
    }

    pa_operation_unref(o);
}

static void sndctx_pulse_sink_info_update_default_cb(
    pa_context*         c,
    const pa_sink_info* i,
    WINTC_UNUSED(int eol),
    void*               userdata
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(userdata);

    if (i == NULL)
    {
        return;
    }

    if (ctx->sink_default_changed)
    {
        ctx->sink_default = wintc_sndapi_output_new(c, i->index);

        g_signal_emit(
            ctx,
            wintc_sndapi_context_signals[SIGNAL_DEFAULT_OUTPUT_CHANGED],
            0
        );
    }

    wintc_sndapi_output_update_from_sink_info(
        ctx->sink_default,
        i
    );
}

static void sndctx_pulse_state_cb(
    pa_context* c,
    void*       userdata
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(userdata);

    gboolean connected_changed = FALSE;

    switch (pa_context_get_state(c))
    {
        case PA_CONTEXT_UNCONNECTED:
            WINTC_LOG_DEBUG("%s", "PA unconnected.");
            connected_changed = TRUE;
            break;

        case PA_CONTEXT_CONNECTING:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA connecting...");
            break;

        case PA_CONTEXT_AUTHORIZING:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA authorizing...");
            break;

        case PA_CONTEXT_SETTING_NAME:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA setting name...");
            break;

        case PA_CONTEXT_READY:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA ready.");
            connected_changed = TRUE;

            pa_context_subscribe(
                c,
                PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER,
                NULL,
                NULL
            );
            pa_context_set_subscribe_callback(
                c,
                sndctx_pulse_subscribe_cb,
                ctx
            );

            wintc_sndapi_context_update_from_pa(ctx);

            break;

        case PA_CONTEXT_FAILED:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA failed.");
            connected_changed = TRUE;
            break;

        case PA_CONTEXT_TERMINATED:
            WINTC_LOG_DEBUG("%s", "SNDAPI: PA terminated.");
            connected_changed = TRUE;
            break;
    }

    if (connected_changed)
    {
        wintc_sndapi_context_emit_connected_changed(ctx);
    }
}

static void sndctx_pulse_subscribe_cb(
    WINTC_UNUSED(pa_context* c),
    pa_subscription_event_type_t t,
    WINTC_UNUSED(uint32_t idx),
    void*                        userdata
)
{
    WinTCSndApiContext* ctx = WINTC_SNDAPI_CONTEXT(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
        case PA_SUBSCRIPTION_EVENT_SERVER:
        case PA_SUBSCRIPTION_EVENT_SINK:
            wintc_sndapi_context_update_from_pa(ctx);
            break;

        default:
            g_warning("%s", "sndapi: unknown event received from PulseAudio");
            break;
    }
}
