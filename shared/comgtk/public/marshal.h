/** @file */

#ifndef __COMGTK_MARSHAL_H__
#define __COMGTK_MARSHAL_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * A GClosureMarshal function for use with signals with handlers that take no
 * arguments and return a boolean.
 */
void wintc_cclosure_marshal_BOOLEAN__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    gpointer      invocation_hint,
    gpointer      marshal_data
);

/**
 * A GClosureMarshal function for use with signals with handlers that take no
 * arguments and return an integer.
 */
void wintc_cclosure_marshal_INT__VOID(
    GClosure*     closure,
    GValue*       return_value,
    guint         n_param_values,
    const GValue* param_values,
    gpointer      invocation_hint,
    gpointer      marshal_data
);

#endif

