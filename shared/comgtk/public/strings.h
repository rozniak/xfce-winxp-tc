/** @file */

#ifndef __COMGTK_STRINGS_H__
#define __COMGTK_STRINGS_H__

#include <glib.h>

/**
 * @def WINTC_STR_TRANSFORM(strvar, strfn)
 *
 * Convenience macro for doing an ASCII transformation in place, use this macro
 * for things like WINTC_STR_TRANSFORM(my_string, g_ascii_tolower).
 */
#define WINTC_STR_TRANSFORM(strvar, strfn) \
{ \
    gchar* wstrtmp = strfn(strvar); \
    g_free(strvar); \
    strvar = wstrtmp; \
}

/**
 * @def WINTC_UTF8_TRANSFORM(strvar, strfn)
 *
 * Convenience macro for doing a UTF-8 transformation in place, use this macro
 * for things like WINTC_UTF8_TRANSFORM(my_string, g_utf8_casefold).
 */
#define WINTC_UTF8_TRANSFORM(strvar, strfn) \
{ \
    gchar* wstrtmp = strfn(strvar, -1); \
    g_free(strvar); \
    strvar = wstrtmp; \
}

//
// PUBLIC FUNCTIONS
//

/**
 * Gets the last component of a path.
 *
 * @param path The path.
 * @return A pointer to the last component of the path.
 */
const gchar* wintc_basename(
    const gchar* path
);

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
 * Duplicates the substring in an array of delimiter separated values by the
 * specified index.
 */
gchar* wintc_strdup_delimited(
    const gchar* str,
    gchar*       delim_str,
    guint        index
);

/**
 * Duplicates part of a string from the start and ending at the first
 * occurrence of the specified delimiter (exclusive, so the delimiter will not
 * be included).
 *
 * @param str The string.
 * @param len The maximum length of str to use, -1 for null terminated string.
 * @param c   The character to look for.
 * @param pos If not NULL, a storage location for position after the delimiter.
 * @return The new string, a complete copy of the string if c wasn't found.
 */
gchar* wintc_strdup_nextchr(
    const gchar*  str,
    gssize        len,
    gunichar      c,
    const gchar** pos
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

/**
 * Extracts a substring from one string to create a new string.
 *
 * @param start A pointer to the start of the substring.
 * @param end   A pointer to the end of the substring or NULL.
 * @return A copy of the substring.
 */
gchar* wintc_substr(
    const gchar* start,
    const gchar* end
);

#endif
