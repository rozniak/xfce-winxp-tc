#ifndef __WINTC_SNDAPI_H__
#define __WINTC_SNDAPI_H__

#include <glib.h>

//
// Output
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
// Context
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
