/** @file */

#ifndef __EXEC_DESKTOP_H__
#define __EXEC_DESKTOP_H__

#include <gio/gdesktopappinfo.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Retrieves the command line from a desktop entry.
 *
 * @param entry The desktop entry.
 * @return The command line.
 */
gchar* wintc_desktop_app_info_get_command(
    GDesktopAppInfo* entry
);

/**
 * Fills in the placeholders in a desktop entry's command line.
 *
 * @param cmdline        The command line.
 * @param name           The translated name of the program.
 * @param icon_name      The icon name.
 * @param entry_path     The path to the desktop entry.
 * @param needs_terminal Specifies if the program requires a terminal.
 * @return The expanded command line.
 */
gchar* wintc_expand_desktop_entry_cmdline(
    const gchar* cmdline,
    const gchar* name,
    const gchar* icon_name,
    const gchar* entry_path,
    gboolean     needs_terminal
);

#endif
