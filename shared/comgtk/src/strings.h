#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
gchar* wintc_str_set_prefix(
    const gchar* str,
    const gchar* prefix
);

gchar* wintc_str_set_suffix(
    const gchar* str,
    const gchar* suffix
);

gint wintc_strstr_count(
    const gchar* haystack,
    const gchar* needle
);

void wintc_strsteal(
    gchar** dest,
    gchar** src
);

gchar* wintc_strsubst(
    const gchar* str,
    const gchar* findwhat,
    const gchar* replacewith
);

#endif
