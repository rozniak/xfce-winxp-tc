#ifndef __EXEC_H__
#define __EXEC_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

gchar* wintc_desktop_app_info_get_command(
    GDesktopAppInfo* entry
);

gboolean wintc_launch_command(
    const gchar* command,
    GError**     out_error
);

gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
);

gchar* wintc_query_mime_handler(
    const gchar*      mime_query,
    GError**          out_error,
    GDesktopAppInfo** out_entry
);

#endif
