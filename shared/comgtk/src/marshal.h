#ifndef __MARSHAL_H__
#define __MARSHAL_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
void wintc_cclosure_marshal_BOOLEAN__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    gpointer      invocation_hint,
    gpointer      marshal_data
);

void wintc_cclosure_marshal_INT__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    gpointer      invocation_hint,
    gpointer      marshal_data
);

#endif

