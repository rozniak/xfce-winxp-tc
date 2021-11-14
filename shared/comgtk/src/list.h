#ifndef __LIST_H__
#define __LIST_H__

//
// PUBLIC FUNCTIONS
//
GList* wintc_list_distinct_append(
    GList*       list,
    gpointer     data,
    GCompareFunc comparer
);

GList* wintc_list_distinct_insert(
    GList*       list,
    gpointer     data,
    gint         position,
    GCompareFunc comparer
);

GList* wintc_list_distinct_prepend(
    GList*       list,
    gpointer     data,
    GCompareFunc comparer
);

gchar* wintc_list_implode_strings(
    GList* list
);

GList* wintc_list_limit(
    GList*   list,
    gint     limit,
    gboolean free_data
);

GList* wintc_list_read_from_string(
    const gchar* str
);

#endif
