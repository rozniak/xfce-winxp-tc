#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <glib.h>

#include "output.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCSndApiContextClass WinTCSndApiContextClass;
typedef struct _WinTCSndApiContext      WinTCSndApiContext;

#define TYPE_WINTC_SNDAPI_CONTEXT            (wintc_sndapi_context_get_type())
#define WINTC_SNDAPI_CONTEXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_SNDAPI_CONTEXT, WinTCSndApiContext))
#define WINTC_SNDAPI_CONTEXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_SNDAPI_CONTEXT, WinTCSndApiContextClass))
#define IS_WINTC_SNDAPI_CONTEXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_SNDAPI_CONTEXT))
#define IS_WINTC_SNDAPI_CONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_SNDAPI_CONTEXT))
#define WINTC_SNDAPI_CONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_SNDAPI_CONTEXT, WinTCSndApiContext))

GType wintc_sndapi_context_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCSndApiContext* wintc_sndapi_context_new(void);

void wintc_sndapi_context_connect(
    WinTCSndApiContext* ctx
);
WinTCSndApiOutput* wintc_sndapi_context_get_default_output(
    WinTCSndApiContext* ctx
);
gboolean wintc_sndapi_context_is_connected(
    WinTCSndApiContext* ctx
);

#endif
