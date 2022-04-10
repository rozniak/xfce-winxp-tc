#ifndef __MIME_H__
#define __MIME_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

gchar* wintc_query_mime_for_file(
    const gchar* filepath,
    GError**     out_error
);

GDesktopAppInfo* wintc_query_mime_handler(
    const gchar* mime_query,
    GError**     out_error
);

#endif
