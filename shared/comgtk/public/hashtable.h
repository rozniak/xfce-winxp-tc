/** @file */

#ifndef __COMGTK_HASHTABLE_H__
#define __COMGTK_HASHTABLE_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Inserts mapping pairs from an array into a hash table.
 *
 * @param hash_table The hash table.
 * @param mappings   The array of mappings pairs.
 * @param arr_size   The total elements in the array (not number of pairs).
 */
void wintc_hash_table_insert_from_array(
    GHashTable* hash_table,
    void**      mappings,
    gsize       arr_size
);

#endif
