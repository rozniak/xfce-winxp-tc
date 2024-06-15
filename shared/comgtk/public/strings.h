/** @file */

#ifndef __COMGTK_STRINGS_H__
#define __COMGTK_STRINGS_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Copies a string, ensuring it has the specified prefix.
 *
 * @param str    The string.
 * @param prefix The desired prefix.
 * @return The new string.
 */
gchar* wintc_str_set_prefix(
    const gchar* str,
    const gchar* prefix
);

/**
 * Copies a string, ensuring it has the specified suffix.
 *
 * @param str The string.
 * @param suffix The desired suffix.
 * @return The new string.
 */
gchar* wintc_str_set_suffix(
    const gchar* str,
    const gchar* suffix
);

/**
 * Duplicates a string and replaces the string at the destination with it - if
 * there was a string at the destination pointer, it will be freed.
 *
 * @param dest Reference to the destination pointer.
 * @param src  Reference to the source pointer.
 */
void wintc_strdup_replace(
    gchar**      dest,
    const gchar* src
);

/**
 * Counts the number of time a string appears within another string.
 *
 * @param haystack The subject string.
 * @param needle   The string to search for.
 * @return The number of times the string appears.
 */
gint wintc_strstr_count(
    const gchar* haystack,
    const gchar* needle
);

/**
 * Transfers a string between two pointers - if the destination pointer already
 * refers to a string it will be freed.
 *
 * @param dest Reference to the destination pointer.
 * @param src  Reference to the source pointer.
 */
void wintc_strsteal(
    gchar** dest,
    gchar** src
);

/**
 * Finds and replaces appearances of a string with another string.
 *
 * @param str         The subject string.
 * @param findwhat    The string to find.
 * @param replacewith The string to use as the replacement.
 * @return A new string with the replacements.
 */
gchar* wintc_strsubst(
    const gchar* str,
    const gchar* findwhat,
    const gchar* replacewith
);

/**
 * Constant version of g_strv_length - returns the length of an array of
 * strings. str_array must not be NULL.
 *
 * @param str_array A null-terminated array of gchar*.
 * @return Length of str_array.
 */
guint wintc_strv_length(
    const gchar** str_array
);

#endif
