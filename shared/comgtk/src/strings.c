#include <glib.h>
#include <string.h>

#include "../public/shorthand.h"
#include "../public/strings.h"

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_basename(
    const gchar* path
)
{
    const gchar* last_sep =
        strrchr(path, G_DIR_SEPARATOR);

    if (last_sep)
    {
        return last_sep + 1;
    }

    return path;
}

gchar* wintc_str_set_prefix(
    const gchar* str,
    const gchar* prefix
)
{
    if (g_str_has_prefix(str, prefix))
    {
        return g_strdup(str);
    }
    else
    {
        return g_strdup_printf(
            "%s%s",
            prefix,
            str
        );
    }
}

gchar* wintc_str_set_suffix(
    const gchar* str,
    const gchar* suffix
)
{
    if (g_str_has_suffix(str, suffix))
    {
        return g_strdup(str);
    }
    else
    {
        return g_strdup_printf(
            "%s%s",
            str,
            suffix
        );
    }
}

gchar* wintc_strdup_delimited(
    const gchar* str,
    gchar*       delim_str,
    guint        index
)
{
    gint         delim_len = strlen(delim_str); 
    const gchar* next      = str;

    for (guint i = 0; i < index; i++)
    {
        if (next != str)
        {
            next += delim_len; // Advance past the delimiter
        }

        next = strstr(next, delim_str);

        if (!next)
        {
            return NULL;
        }
    }

    next += delim_len;

    return wintc_substr(next, strstr(next, delim_str));
}

gchar* wintc_strdup_nextchr(
    const gchar*  str,
    gssize        len,
    gunichar      c,
    const gchar** pos
)
{
    const gchar* end;

    if (!str)
    {
        WINTC_SAFE_REF_SET(pos, NULL);
        return NULL;
    }

    end = g_utf8_strchr(str, len, c);

    if (pos)
    {
        WINTC_SAFE_REF_SET(pos, end ? end + 1 : NULL);
    }

    if (!end)
    {
        return g_strdup(str);
    }

    // Allocate new string
    //
    gint   diff = end - str;
    gchar* buf  = g_malloc0(diff + 1);

    memcpy(buf, str, diff);

    return buf;
}

void wintc_strdup_replace(
    gchar**      dest,
    const gchar* src
)
{
    if (*dest != NULL)
    {
        g_free(g_steal_pointer(dest));
    }

    *dest = g_strdup(src);
}

gint wintc_strstr_count(
    const gchar* haystack,
    const gchar* needle
)
{
    gint         advance = strlen(needle);
    gint         count   = 0;
    const gchar* pchar   = haystack;

    while (pchar != NULL)
    {
        pchar = strstr(pchar, needle);

        if (pchar != NULL)
        {
            count++;
            pchar += advance;
        }
    }

    return count;
}

void wintc_strsteal(
    gchar** dest,
    gchar** src
)
{
    if (*dest != NULL)
    {
        g_free(g_steal_pointer(dest));
    }

    *dest = *src;
    *src = NULL;
}

gchar* wintc_strsubst(
    const gchar* str,
    const gchar* findwhat,
    const gchar* replacewith
)
{
    gchar*       buffer;
    gint         findsize     = strlen(findwhat);
    gint         replacecount = wintc_strstr_count(str, findwhat);
    gint         replacesize  = strlen(replacewith);
    gchar*       pbuffer;
    const gchar* pchar        = str;
    const gchar* pnext        = NULL;
    gint         required;
    gint         strsize      = strlen(str);
    gint         tocopy;

    // Work out how much space we need
    //
    required = strsize - (findsize * replacecount) + (replacesize * replacecount) + 1;
    buffer   = g_new0(gchar, required);

    pbuffer = buffer;

    // Build the string now
    //
    while (pchar != NULL)
    {
        // Find where the replacement begins
        //
        pnext = strstr(pchar, findwhat);

        if (pnext == NULL)
        {
            // No more? Add the rest of the string
            //
            tocopy = strsize - (pchar - str);

            memcpy(pbuffer, pchar, tocopy);
        }
        else
        {
            // Add the next chunk with the replacement and advance
            //
            tocopy = pnext - pchar;

            memcpy(pbuffer, pchar, tocopy);
            pbuffer += tocopy;

            memcpy(pbuffer, replacewith, replacesize);
            pbuffer += replacesize;

            pnext += findsize;
        }

        pchar = pnext;
    }

    return buffer;
}

guint wintc_strv_length(
    const gchar** str_array
)
{
    guint i;

    for (i = 0; str_array[i]; i++);

    return i;
}

gchar* wintc_substr(
    const gchar* start,
    const gchar* end
)
{
    if (!end)
    {
        end = start + strlen(start);
    }

    if (end < start)
    {
        g_critical("substr: invalid substring requested");
        return NULL;
    }

    gchar* buf = g_malloc0(end - start + 1);

    if (start != end)
    {
        memcpy(buf, start, end - start);
    }

    return buf;
}
