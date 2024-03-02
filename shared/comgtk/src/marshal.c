#include <glib.h>
#include <gtk/gtk.h>

#include "../public/marshal.h"
#include "../public/shorthand.h"

//
// PUBLIC FUNCTIONS
//
void wintc_cclosure_marshal_BOOLEAN__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    WINTC_UNUSED(gpointer invocation_hint),
    gpointer      marshal_data
)
{
    typedef gboolean (*GMarshalFunc_BOOLEAN__VOID) (
        gpointer data1,
        gpointer data2
    );

    GMarshalFunc_BOOLEAN__VOID callback;
    GCClosure*                 cc;
    gpointer                   data1;
    gpointer                   data2;
    gboolean                   v_return;

    cc = (GCClosure*) closure;

    g_return_if_fail(n_param_values == 1);

    if (G_CCLOSURE_SWAP_DATA(closure))
    {
        data1 = closure->data;
        data2 = g_value_peek_pointer(param_values + 0);
    }
    else
    {
        data1 = g_value_peek_pointer(param_values + 0);
        data2 = closure->data;
    }

    callback =
        (GMarshalFunc_BOOLEAN__VOID)
        (marshal_data ? marshal_data : cc->callback);

    v_return = callback(data1, data2);

    g_value_set_boolean(return_value, v_return);
}

void wintc_cclosure_marshal_INT__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    WINTC_UNUSED(gpointer invocation_hint),
    gpointer      marshal_data
)
{
    typedef gint (*GMarshalFunc_INT__VOID) (
        gpointer data1,
        gpointer data2
    );

    GMarshalFunc_INT__VOID callback;
    GCClosure*             cc;
    gpointer               data1;
    gpointer               data2;
    gboolean               v_return;

    cc = (GCClosure*) closure;

    g_return_if_fail(n_param_values == 1);

    if (G_CCLOSURE_SWAP_DATA(closure))
    {
        data1 = closure->data;
        data2 = g_value_peek_pointer(param_values + 0);
    }
    else
    {
        data1 = g_value_peek_pointer(param_values + 0);
        data2 = closure->data;
    }

    callback =
        (GMarshalFunc_INT__VOID)
        (marshal_data ? marshal_data : cc->callback);

    v_return = callback(data1, data2);

    g_value_set_int(return_value, v_return);
}

