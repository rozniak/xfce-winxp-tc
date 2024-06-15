#include <glib.h>

#include "../public/memory.h"

//
// PUBLIC FUNCTIONS
//
void wintc_freev(
    gpointer       mem,
    GDestroyNotify destroy
)
{
    void** arr = (void**) mem;

    for (guint i = 0; arr[i]; i++)
    {
        destroy(arr[i]);
    }

    g_free(mem);
}

void wintc_freenv(
    gpointer       mem,
    guint          n_elements,
    GDestroyNotify destroy
)
{
    void** arr = (void**) mem;

    for (guint i = 0; i < n_elements; i++)
    {
        destroy(arr[i]);
    }

    g_free(mem);
}
