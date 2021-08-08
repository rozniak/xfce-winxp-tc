#ifndef __UTIL_H__
#define __UTIL_H__

#include <garcon/garcon.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>

void connect_widget_list_signals(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
);

void display_not_implemented_error();

gchar* g_desktop_app_info_get_command_expanded(
    GDesktopAppInfo* entry
);

GDesktopAppInfo* g_desktop_app_info_new_from_scheme(
    const gchar* scheme
);

gchar* g_str_set_suffix(
    const gchar* str,
    const gchar* suffix
);

gchar* garcon_menu_item_get_command_expanded(
    GarconMenuItem* item
);

void gtk_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
);

void menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
);

void menu_shell_select_on_enter(
    GtkWidget*    widget,
    GdkEvent*     event,
    GtkMenuShell* menu_shell
);

void report_g_error_and_clear(
    GError** error
);

gchar** true_shell_parse_argv(
    const gchar* cmdline
);

#endif
