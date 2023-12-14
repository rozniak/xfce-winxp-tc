#include <glib.h>
#include <pulse/pulseaudio.h>

#include "volcvt.h"

//
// INTERNAL FUNCTIONS
//
gdouble wintc_sndapi_cvt_pa_volume_to_percent(
    pa_volume_t volume
)
{
    gdouble adjusted_vol;
    gdouble range;

    adjusted_vol = (gdouble) (volume - PA_VOLUME_MUTED);
    range        = (gdouble) (PA_VOLUME_NORM - PA_VOLUME_MUTED);

    return adjusted_vol / range;
}

pa_volume_t wintc_sndapi_cvt_percent_to_pa_volume(
    gdouble pct
)
{
    gdouble     range;
    pa_volume_t scaled;

    range  = (gdouble) (PA_VOLUME_NORM - PA_VOLUME_MUTED);
    scaled = (pa_volume_t) (pct * range);

    return scaled + PA_VOLUME_MUTED;
}
