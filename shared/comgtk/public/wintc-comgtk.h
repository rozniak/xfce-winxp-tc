#ifndef __WINTC_COMGTK_H__
#define __WINTC_COMGTK_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// Debugging
//
#define WINTC_ENVVAR_DEBUG_LOGGING "WINDEBUG"
#define WINTC_LOG_DEBUG(...) if (getenv(WINTC_ENVVAR_DEBUG_LOGGING)) { g_message(__VA_ARGS__); }

//
// Shorthand
//
#define WINTC_SAFE_REF_CLEAR(ref) if (ref != NULL) { *ref = NULL; }
#define WINTC_SAFE_REF_SET(ref, value) if (ref != NULL) { *ref = value; }

#define WINTC_UNUSED(arg) __attribute__((unused)) arg

//
// gchar-related
//
#define WINTC_GCHAR_BUFFER_SIZE sizeof(gchar) * 255

//
// Default procedures
//
void wintc_menu_shell_deselect_on_leave(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

void wintc_menu_shell_select_on_enter(
    GtkWidget*    widget,
    WINTC_UNUSED(GdkEvent* event),
    GtkMenuShell* menu_shell
);

//
// Error-related
//
#define WINTC_GENERAL_ERROR wintc_general_error_quark()

typedef enum
{
    WINTC_GENERAL_ERROR_NOTIMPL
} WinTCGeneralError;

void wintc_display_error_and_clear(
    GError** error
);

GQuark wintc_general_error_quark(void);

void wintc_log_error_and_clear(
    GError** error
);

void wintc_nice_error_and_clear(
    GError** error
);

//
// List-related
//
GList* wintc_list_distinct_append(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

GList* wintc_list_distinct_insert(
    GList*         list,
    gpointer       data,
    gint           position,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

GList* wintc_list_distinct_prepend(
    GList*         list,
    gpointer       data,
    GCompareFunc   comparer,
    GDestroyNotify free_func
);

gchar* wintc_list_implode_strings(
    GList* list
);

GList* wintc_list_limit(
    GList*         list,
    gint           limit,
    GDestroyNotify free_func
);

GList* wintc_list_read_from_string(
    const gchar* str
);

//
// Message Box-related
//
gint wintc_messagebox_show(
    GtkWindow*     parent,
    const gchar*   text,
    const gchar*   caption,
    GtkButtonsType buttons,
    GtkMessageType type
);

//
// Profile-related
//
#define WINTC_COMPONENT_SHELL "shell"

gboolean wintc_profile_ensure_exists(
    const gchar* component,
    GError**     out_error
);

gchar* wintc_profile_get_path(
    const gchar* component,
    const gchar* filename
);

gboolean wintc_profile_get_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar**      contents,
    gsize*       length,
    GError**     error
);

gboolean wintc_profile_set_file_contents(
    const gchar* component,
    const gchar* filename,
    gchar*       contents,
    gssize       length,
    GError**     error
);

//
// Signal-related
//
void wintc_signal_connect_list(
    GList*       widgets,
    const gchar* signal_name,
    GCallback    cb,
    gpointer     user_data
);

//
// Strings-related
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

//
// Styles-related
//
void wintc_widget_add_css(
    GtkWidget*   widget,
    const gchar* css
);

void wintc_widget_add_style_class(
    GtkWidget*   widget,
    const gchar* class_name
);

//
// Version-related
//
gboolean wintc_build_is_debug();
gchar* wintc_get_build_tag(void);

//
// Window-related
//
void wintc_focus_window(
    GtkWindow* window
);

#endif
