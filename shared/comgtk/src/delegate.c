#include <glib.h>

#include "../public/delegate.h"
#include "../public/shorthand.h"

//
// PUBLIC FUNCTIONS
//
gpointer wintc_copyfunc_strdup(
    gconstpointer src,
    WINTC_UNUSED(gpointer user_data)
)
{
    return g_strdup((const gchar*) src);
}
