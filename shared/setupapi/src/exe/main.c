#include <glib.h>
#include <stdlib.h>
#include <wintc/comgtk.h>

#include "install.h"

//
// STATIC DATA
//
static gboolean S_OPT_MODE_INSTALL = FALSE;
static gchar**  S_OPT_FILES        = NULL;

static GOptionEntry S_OPTIONS[] = {
    {
        "install",
        'i',
        0,
        G_OPTION_ARG_NONE,
        &S_OPT_MODE_INSTALL,
        "Install packages.",
        NULL
    },
    {
        G_OPTION_REMAINING,
        0,
        0,
        G_OPTION_ARG_STRING_ARRAY,
        &S_OPT_FILES,
        "Package names.",
        NULL
    }
};

//
// ENTRY POINT
//
int main(
    int   argc,
    char* argv[]
)
{
    static GOptionContext* ctx = NULL;

    GError* error = NULL;

    ctx = g_option_context_new("- WinTC Package Management");

    g_option_context_add_main_entries(ctx, S_OPTIONS, NULL);

    if (!g_option_context_parse(ctx, &argc, &argv, &error))
    {
        wintc_log_error_and_clear(&error);
        return EXIT_FAILURE;
    }

    // Determine what needs doing
    //
    int status;

    if (S_OPT_MODE_INSTALL)
    {
        // Resolve any paths - fixes issues with package managers that do not
        // like relative paths
        //
        for (gchar** iter = S_OPT_FILES; *iter; iter++)
        {
            if (g_file_test(*iter, G_FILE_TEST_EXISTS))
            {
                gchar* tmp = g_canonicalize_filename(*iter, NULL);

                g_free(*iter);
                *iter = tmp;
            }
        }

        status = wintc_setupapi_exec_install(S_OPT_FILES);

        g_strfreev(S_OPT_FILES);

        return status;
    }

    return EXIT_FAILURE;
}
