#include <glib.h>

#include "../public/hashtable.h"

//
// PUBLIC FUNCTIONS
//
void wintc_hash_table_insert_from_array(
    GHashTable* hash_table,
    void**      mappings,
    gsize       arr_size
)
{
    for (gsize i = 0; i < arr_size; i += 2)
    {
        g_hash_table_insert(
            hash_table,
            mappings[i],
            mappings[i + 1]
        );
    }
}
