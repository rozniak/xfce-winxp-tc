#include <glib.h>

#include "../public/regex.h"
#include "../public/shorthand.h"

//
// STATIC DATA
//
static GRegex* s_regex_uri_scheme = NULL;

//
// PUBLIC FUNCTIONS
//
const GRegex* wintc_regex_uri_scheme(
    GError** error
)
{
    WINTC_SAFE_REF_CLEAR(error);

    if (!s_regex_uri_scheme)
    {
        s_regex_uri_scheme =
            g_regex_new(
                "^([A-Za-z-]+)://",
                0,
                0,
                error
            );
    }

    return s_regex_uri_scheme;
}
