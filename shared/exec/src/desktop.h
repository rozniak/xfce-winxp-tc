#ifndef __DESKTOP_H__
#define __DESKTOP_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

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

#endif
