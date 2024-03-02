/** @file */

#ifndef __SNDAPI_CONTEXT_H__
#define __SNDAPI_CONTEXT_H__

#include <glib.h>

#include "output.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSndApiContextClass WinTCSndApiContextClass;

/**
 * Represents a context for managing system audio.
 */
typedef struct _WinTCSndApiContext WinTCSndApiContext;

#define WINTC_TYPE_SNDAPI_CONTEXT            (wintc_sndapi_context_get_type())
#define WINTC_SNDAPI_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SNDAPI_CONTEXT, WinTCSndApiContext))
#define WINTC_SNDAPI_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SNDAPI_CONTEXT, WinTCSndApiContextClass))
#define IS_WINTC_SNDAPI_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SNDAPI_CONTEXT))
#define IS_WINTC_SNDAPI_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SNDAPI_CONTEXT))
#define WINTC_SNDAPI_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SNDAPI_CONTEXT, WinTCSndApiContext))

GType wintc_sndapi_context_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCSndApiContext.
 *
 * @return The new WinTCSndApiContext instance.
 */
WinTCSndApiContext* wintc_sndapi_context_new(void);

/**
 * Establishes a connection to the underlying system audio provider.
 *
 * @param ctx The sound API context.
 */
void wintc_sndapi_context_connect(
    WinTCSndApiContext* ctx
);

/**
 * Retrieves the current default output device.
 *
 * @param ctx The sound API context.
 * @return The current default output device.
 */
WinTCSndApiOutput* wintc_sndapi_context_get_default_output(
    WinTCSndApiContext* ctx
);

/**
 * Determines if the context has an active connection to the underlying system
 * audio provider.
 *
 * @param ctx The sound API context.
 * @return True if a connection is present.
 */
gboolean wintc_sndapi_context_is_connected(
    WinTCSndApiContext* ctx
);

#endif
