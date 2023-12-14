#ifndef __VOLCVT_H__
#define __VOLCVT_H__

#include <glib.h>
#include <pulse/pulseaudio.h>

//
// INTERNAL FUNCTIONS
//
gdouble wintc_sndapi_cvt_pa_volume_to_percent(
    pa_volume_t volume
);
pa_volume_t wintc_sndapi_cvt_percent_to_pa_volume(
    gdouble pct
);

#endif
