#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include <glib.h>
#include <pulse/pulseaudio.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSndApiOutputClass WinTCSndApiOutputClass;
typedef struct _WinTCSndApiOutput      WinTCSndApiOutput;

#define TYPE_WINTC_SNDAPI_OUTPUT            (wintc_sndapi_output_get_type())
#define WINTC_SNDAPI_OUTPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_SNDAPI_OUTPUT, WinTCSndApiOutput))
#define WINTC_SNDAPI_OUTPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_SNDAPI_OUTPUT, WinTCSndApiOutputClass))
#define IS_WINTC_SNDAPI_OUTPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_SNDAPI_OUTPUT))
#define IS_WINTC_SNDAPI_OUTPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_SNDAPI_OUTPUT))
#define WINTC_SNDAPI_OUTPUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_SNDAPI_OUTPUT, WinTCSndApiOutput))

GType wintc_sndapi_output_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
gdouble wintc_sndapi_output_get_volume(
    WinTCSndApiOutput* output
);
gboolean wintc_sndapi_output_is_muted(
    WinTCSndApiOutput* output
);
void wintc_sndapi_output_set_muted(
    WinTCSndApiOutput* output,
    gboolean           muted
);
void wintc_sndapi_output_set_volume(
    WinTCSndApiOutput* output,
    gdouble            new_volume
);

//
// INTERNAL FUNCTIONS
//
WinTCSndApiOutput* wintc_sndapi_output_new( // Private ctor!!
    pa_context* pulse_context,
    uint32_t    sink_id
);

void wintc_sndapi_output_update_from_sink_info(
    WinTCSndApiOutput*  output,
    const pa_sink_info* i
);

#endif
