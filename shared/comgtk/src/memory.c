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

void wintc_memcpy_ref(
    void*       dst_buf,
    glong       offset,
    const void* src_buf,
    size_t      n
)
{
    if (!dst_buf)
    {
        return;
    }

    memcpy(dst_buf + offset, src_buf, n);
}
