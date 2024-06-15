#include <gio/gio.h>
#include <glib.h>

#include "../public/application.h"
#include "../public/strings.h"

//
// PUBLIC FUNCTIONS
//
GFile** wintc_application_command_line_get_files(
    GApplicationCommandLine* command_line,
    guint*                   n_files
)
{
    const gchar** filenames = NULL;
    GFile**       files     = NULL;

    g_variant_dict_lookup(
        g_application_command_line_get_options_dict(command_line),
        G_OPTION_REMAINING,
        "^a&ay",
        &filenames
    );

    if (
        filenames != NULL &&
        (*n_files = wintc_strv_length(filenames)) > 0
    )
    {
        // Prepare the files
        //
        files = g_new(GFile*, *n_files);

        for (guint i = 0; i < *n_files; i++)
        {
            files[i] =
                g_application_command_line_create_file_for_arg(
                    command_line,
                    filenames[i]
                );
        }
    }

    g_free(filenames);

    return files;
}
