#include <garcon/garcon.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "util.h"

//
// FORWARD DECLARATIONS
//
static gchar* expand_commandline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
);

//
// PUBLIC FUNCTIONS
//
void connect_widget_list_signals(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
)
{
    GList* li;

    for (li = widgets; li != NULL; li = li->next)
    {
        g_assert(GTK_IS_WIDGET(li->data));

        g_signal_connect(
            G_OBJECT(li->data),
            signal_name,
            cb,
            user_data
        );
    }
}

void display_not_implemented_error()
{
    GtkWidget* dialog =
        gtk_message_dialog_new(
            NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK,
            "Sorry, this feature has not been implemented yet!"
        );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

gchar* g_desktop_app_info_get_command_expanded(
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
            expand_commandline(
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

GDesktopAppInfo* g_desktop_app_info_new_from_scheme(
    const gchar* scheme
)
{
    gchar* cmd =
        g_strconcat(
            "xdg-mime query default x-scheme-handler/",
            scheme,
            NULL
        );

    GDesktopAppInfo* app_info   = NULL;
    gchar*           cmd_output = NULL;
    GError*          error      = NULL;
    gchar*           filename   = NULL;

    if (g_spawn_command_line_sync(cmd, &cmd_output, NULL, NULL, &error))
    {
        if (cmd_output != NULL)
        {
            filename = g_utf8_substring(
                           cmd_output,
                           0,
                           g_utf8_strlen(cmd_output, -1) - 1
                       );
            app_info = g_desktop_app_info_new(
                           filename
                       );
        }
    }

    if (app_info != NULL)
    {
        g_free(cmd_output);
        g_free(filename);
    }

    g_free(cmd);

    if (error != NULL)
    {
        report_g_error_and_clear(&error);
    }

    return app_info;
}

gchar* g_str_set_suffix(
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
        return g_strconcat(
            str,
            suffix,
            NULL
        );
    }
}

gchar* garcon_menu_item_get_command_expanded(
    GarconMenuItem* item
)
{
    const gchar* raw_cmd = garcon_menu_item_get_command(item);

    if (raw_cmd == NULL)
    {
        return NULL;
    }

    return expand_commandline(
        raw_cmd,
        garcon_menu_item_get_name(item),
        garcon_menu_item_get_icon_name(item),
        garcon_menu_item_get_path(item),
        garcon_menu_item_requires_terminal(item)
    );
}

void gtk_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
)
{
    GtkStyleContext* styles = gtk_widget_get_style_context(widget);

    gtk_style_context_add_class(styles, class_name);
}

void menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
)
{
    if (gtk_menu_item_get_submenu(GTK_MENU_ITEM(widget)) != NULL)
    {
        return;
    }

    gtk_menu_shell_deactivate(menu_shell);
}

void menu_shell_select_on_enter(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
)
{
    gtk_menu_shell_select_item(
        menu_shell,
        widget
    );
}

void report_g_error_and_clear(
    GError** error
)
{
    g_error((*error)->message);
    g_clear_error(error);
}

gchar** true_shell_parse_argv(
    const gchar* cmdline
)
{
    gchar** argv;
    GError* error = NULL;

    // Parse cmdline into argv
    //
    g_shell_parse_argv(
        cmdline,
        NULL,
        &argv,
        &error
    );

    if (error != NULL)
    {
        report_g_error_and_clear(&error);
    }

    // Resolve path for executable (we might only have the name)
    //
    gchar* tmp = argv[0];

    argv[0] = g_find_program_in_path(tmp);

    g_free(tmp);

    return argv;
}

//
// HELPERS
//
static gchar* expand_commandline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
)
{
    GString*     expanded = g_string_sized_new(250);
    const gchar* iter;

    if (needs_terminal)
    {
        g_string_append(
            expanded,
            "exo-open --launch TerminalEmulator "
        );
    }

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
                        g_string_append(
                            expanded,
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
