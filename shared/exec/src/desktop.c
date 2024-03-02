#include <gio/gdesktopappinfo.h>
#include <glib.h>

#include "../public/desktop.h"

//
// PUBLIC FUNCTIONS
//
gchar* wintc_desktop_app_info_get_command(
    GDesktopAppInfo* entry
)
{
    GAppInfo* app_info = G_APP_INFO(entry);

    const gchar* cmd_line = g_app_info_get_commandline(app_info);
    const gchar* exe_path = g_app_info_get_executable(app_info);

    if (cmd_line != NULL)
    {
        gchar* expanded;
        gchar* icon_name = g_path_get_basename(exe_path);

        expanded =
            wintc_expand_desktop_entry_cmdline(
                cmd_line,
                g_app_info_get_name(app_info),
                icon_name,
                g_desktop_app_info_get_filename(entry),
                FALSE
            );

        g_free(icon_name);

        return expanded;
    }
    else
    {
        return g_strdup(exe_path);
    }
}

gchar* wintc_expand_desktop_entry_cmdline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
)
{
    GString* expanded = g_string_sized_new(250);

    if (needs_terminal)
    {
        g_string_append(
            expanded,
            "exo-open --launch TerminalEmulator "
        );
    }

    // Iterate through cmdline character by character to expand shortcodes
    //
    const gchar* iter;

    for (iter = cmdline; *iter != '\0'; iter++)
    {
        if (
            iter[0] == '%' &&
            iter[1] != '\0'
        )
        {
            switch (*++iter)
            {
                case 'c':
                    if (name != NULL)
                    {
                        g_string_append(
                            expanded,
                            name
                        );
                    }

                    break;

                case 'i':
                    if (icon_name != NULL)
                    {
                        g_string_append_printf(
                            expanded,
                            "--icon %s",
                            icon_name
                        );
                    }

                    break;

                case 'k':
                    g_string_append(
                        expanded,
                        entry_path
                    );

                    break;

                case '%':
                    g_string_append_c(expanded, '%');
                    break;
            }
        }
        else
        {
            g_string_append_c(expanded, *iter);
        }
    }

    return g_string_free(expanded, FALSE);
}
