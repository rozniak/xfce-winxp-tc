#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
gint wintc_strstr_count(
    const gchar* haystack,
    const gchar* needle
);

gboolean wintc_strsteal(
    gchar** dest,
    gchar** src
);

gchar* wintc_strsubst(
    const gchar* str,
    const gchar* findwhat,
    const gchar* replacewith
);

#endif
