/** @file */

#ifndef __SNDAPI_OUTPUT_H__
#define __SNDAPI_OUTPUT_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSndApiOutputClass WinTCSndApiOutputClass;

/**
 * Represents an audio output device.
 */
typedef struct _WinTCSndApiOutput WinTCSndApiOutput;

#define WINTC_TYPE_SNDAPI_OUTPUT            (wintc_sndapi_output_get_type())
#define WINTC_SNDAPI_OUTPUT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SNDAPI_OUTPUT, WinTCSndApiOutput))
#define WINTC_SNDAPI_OUTPUT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SNDAPI_OUTPUT, WinTCSndApiOutputClass))
#define IS_WINTC_SNDAPI_OUTPUT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SNDAPI_OUTPUT))
#define IS_WINTC_SNDAPI_OUTPUT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SNDAPI_OUTPUT))
#define WINTC_SNDAPI_OUTPUT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SNDAPI_OUTPUT, WinTCSndApiOutput))

GType wintc_sndapi_output_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieves the current volume of the specified output.
 *
 * @param output The audio output.
 * @return The volume of the output.
 */
gdouble wintc_sndapi_output_get_volume(
    WinTCSndApiOutput* output
);

/**
 * Determines whether the specified output is muted.
 *
 * @param output The audio output.
 * @return True if the output is muted.
 */
gboolean wintc_sndapi_output_is_muted(
    WinTCSndApiOutput* output
);

/**
 * Sets the mute state on the specified output.
 *
 * @param output The audio output.
 * @param muted  True to mute the output.
 */
void wintc_sndapi_output_set_muted(
    WinTCSndApiOutput* output,
    gboolean           muted
);

/**
 * Sets the current volume on the specified output.
 *
 * @param output     The audio output.
 * @param new_volume The new volume level.
 */
void wintc_sndapi_output_set_volume(
    WinTCSndApiOutput* output,
    gdouble            new_volume
);

#endif
