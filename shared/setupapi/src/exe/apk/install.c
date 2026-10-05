#include <glib.h>
#include <stdlib.h>
#include <wintc/comgtk.h>

#include "../install.h"

//
// PUBLIC FUNCTIONS
//
gint wintc_setupapi_exec_install(
    WINTC_UNUSED(gchar** packages)
)
{
    g_print("ERR %s\n", "not implemented for apk");
    return EXIT_FAILURE;
}
