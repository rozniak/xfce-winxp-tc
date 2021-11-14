#include <glib.h>

#include "list.h"

//
// PUBLIC FUNCTIONS
//
GList* wintc_list_distinct_append(
    GList*       list,
    gpointer     data,
    GCompareFunc comparer
)
{
    return wintc_list_distinct_insert(
        list,
        data,
        -1,
        comparer
    );
}

GList* wintc_list_distinct_insert(
    GList*       list,
    gpointer     data,
    gint         position,
    GCompareFunc comparer
)
{
    GList* search_result =
        g_list_find_custom(
            list,
            data,
            comparer
        );

    if (search_result != NULL)
    {
        return g_list_first(list);
    }

    return g_list_insert(
        list,
        data,
        position
    );
}

GList* wintc_list_distinct_prepend(
    GList*       list,
    gpointer     data,
    GCompareFunc comparer
)
{
    return wintc_list_distinct_insert(
        list,
        data,
        0,
        comparer
    );
}

gchar* wintc_list_implode_strings(
    GList* list
)
{
    gboolean first = TRUE;
    GList*   iter;
    GString* str   = g_string_new(NULL);

    iter = list;

    while (iter != NULL)
    {
        if (first)
        {
            g_string_append_printf(
                str,
                "%s",
                iter->data
            );

            first = FALSE;
        }
        else
        {
            g_string_append_printf(
                str,
                "\n%s",
                iter->data
            );
        }

        iter = g_list_next(iter);
    }

    return g_string_free(str, FALSE);
}

GList* wintc_list_limit(
    GList*   list,
    gint     limit,
    gboolean free_data
)
{
    GList* res       = g_list_first(list);
    gint   oversized; 
    guint  size      = g_list_length(list);
    GList* to_remove;

    oversized = size - limit;

    for (gint i = 0; i < oversized; i++)
    {
        to_remove = g_list_last(list);

        if (free_data)
        {
            g_free(to_remove->data);
        }

        res =
            g_list_delete_link(
                list,
                to_remove
            );
    }

    return res;
}

GList* wintc_list_read_from_string(
    const gchar* str
)
{
    guint   count;
    gchar** lines = NULL;
    GList*  list  = NULL;

    lines = g_strsplit(str, "\n", -1);
    count = g_strv_length(lines);

    for (int i = 0; i < count; i++)
    {
        list =
            g_list_append(
                list,
                g_strdup(lines[i])
            );
    }

    g_strfreev(lines);
    
    return list;
}
