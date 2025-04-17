/** @file */

#ifndef __COMGTK_MEMORY_H__
#define __COMGTK_MEMORY_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Frees a null-terminated array - the provided function will be called to free
 * each of the array elements.
 *
 * @param mem     The array.
 * @param destroy The function for freeing an individual element.
 */
void wintc_freev(
    gpointer       mem,
    GDestroyNotify destroy
);

/**
 * Frees an array of the specified size - the provided function will be called
 * to free each of the array elements.
 *
 * @param mem        The array.
 * @param n_elements The number of elements in the array.
 * @param destroy    The function for freeing an individual element.
 */
void wintc_freenv(
    gpointer       mem,
    guint          n_elements,
    GDestroyNotify destroy
);

/**
 * Convenience function for using memcpy where potentially the destination
 * buffer is NULL, in which case nothing will be done.
 *
 * @param dst_buf The reference to the buffer to copy into.
 * @param offset  The offset into the destination buffer to copy at.
 * @param src_buf The source buffer.
 * @param n       The number of bytes to be copied.
 */
void wintc_memcpy_ref(
    void*       dst_buf,
    glong       offset,
    const void* src_buf,
    size_t      n
);

#endif
