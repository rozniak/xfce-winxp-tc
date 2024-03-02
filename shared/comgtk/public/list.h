/** @file */

#ifndef __COMGTK_LIST_H__
#define __COMGTK_LIST_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Appends an item to the end of a list, if an item of equal value already
 * exists in the list it will be removed and freed.
 *
 * @param list      The list.
 * @param data      The item to append.
 * @param comparer  The function for comparing existing items to the new one.
 * @param free_func The function for freeing an item, if necessary.
 * @return The list.
 */
GList* wintc_list_distinct_append(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

/**
 * Inserts an item at a specific index to a list, if an item of equal value
 * already exists in the list it will be removed and freed.
 *
 * @param list      The list.
 * @param data      The item to insert.
 * @param position  The index in the list, -1 to append to the end of the list.
 * @param comparer  The function for comparing existing items to the new one.
 * @param free_func The function for freeing an item, if necessary.
 * @return The list.
 */
GList* wintc_list_distinct_insert(
    GList*         list,
    gpointer       data,
    gint           position,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

/**
 * Prepends an item at the start of a list, if an item of equal value already
 * exists in the list it will be removed and freed.
 *
 * @param list      The list.
 * @param data      The item to prepend.
 * @param comparer  The function for comparing existing items to the new one.
 * @param free_func The function for freeing an item, if necessary.
 * @return The list.
 */
GList* wintc_list_distinct_prepend(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

/**
 * Joins all strings together in the list into one single string.
 *
 * @param list The list.
 * @return The compiled string.
 */
gchar* wintc_list_implode_strings(
    GList* list
);

/**
 * Clamps the size of a list, removing and freeing items that exist beyond that
 * limit.
 *
 * @param list      The list.
 * @param limit     The max items in the list.
 * @param free_func The function for freeing an item, if necessary.
 * @return The list.
 */
GList* wintc_list_limit(
    GList*         list,
    gint           limit,
    GDestroyNotify free_func
);

/**
 * Creates a list out of the lines in a string.
 *
 * @param str The string.
 * @return The newly created list whose contents are the lines of the string.
 */
GList* wintc_list_read_from_string(
    const gchar* str
);

#endif
