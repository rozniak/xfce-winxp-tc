#ifndef __OUTPUT_PRIV_H__
#define __OUTPUT_PRIV_H__

#include <glib.h>
#include <pulse/pulseaudio.h>

#include "../public/output.h"

//
// INTERNAL FUNCTIONS
//
WinTCSndApiOutput* wintc_sndapi_output_new(
    pa_context* pulse_context,
    uint32_t    sink_id
);

void wintc_sndapi_output_update_from_sink_info(
    WinTCSndApiOutput*  output,
    const pa_sink_info* i
);

#endif
