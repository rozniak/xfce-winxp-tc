#ifndef __WINTC_EXEC_H__
#define __WINTC_EXEC_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

//
// Actions
//
typedef enum
{
    WINTC_ACTION_MYDOCS,
    WINTC_ACTION_MYRECENTS,
    WINTC_ACTION_MYPICS,
    WINTC_ACTION_MYMUSIC,
    WINTC_ACTION_MYCOMP,
    WINTC_ACTION_CONTROL,
    WINTC_ACTION_MIMEMGMT,
    WINTC_ACTION_CONNECTTO,
    WINTC_ACTION_PRINTERS,
    WINTC_ACTION_HELP,
    WINTC_ACTION_SEARCH,
    WINTC_ACTION_RUN,
    WINTC_ACTION_LOGOFF,
    WINTC_ACTION_SHUTDOWN
} WinTCAction;

gboolean wintc_launch_action(
    WinTCAction action_id,
    GError**    out_error
);

//
// Desktop entries
//
gchar* wintc_desktop_app_info_get_command(
    GDesktopAppInfo* entry
);

gchar* wintc_expand_desktop_entry_cmdline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
);

//
// Execution
//
gboolean wintc_launch_command(
    const gchar* command,
    GError**     out_error
);

//
// MIME types
//
gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
);

GDesktopAppInfo* wintc_query_mime_handler(
    const gchar* mime_query,
    GError**     out_error
);

#endif
