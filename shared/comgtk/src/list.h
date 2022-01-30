#ifndef __LIST_H__
#define __LIST_H__

//
// PUBLIC FUNCTIONS
//
GList* wintc_list_distinct_append(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

GList* wintc_list_distinct_insert(
    GList*         list,
    gpointer       data,
    gint           position,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

GList* wintc_list_distinct_prepend(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

gchar* wintc_list_implode_strings(
    GList* list
);

GList* wintc_list_limit(
    GList*         list,
    gint           limit,
    GDestroyNotify free_func
);

GList* wintc_list_read_from_string(
    const gchar* str
);

#endif
