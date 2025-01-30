/** @file */

#ifndef __COMGTK_DELEGATE_H__
#define __COMGTK_DELEGATE_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * GCopyFunc stub for g_strdup.
 */
gpointer wintc_copyfunc_strdup(
    gconstpointer src,
    gpointer      user_data
);

#endif
