#include <garcon/garcon.h>
#include <glib.h>
#include <wintc-exec.h>

#include "util.h"

//
// PUBLIC FUNCTIONS
//
gchar* garcon_menu_item_get_command_expanded(
    GarconMenuItem* item
)
{
    const gchar* raw_cmd = garcon_menu_item_get_command(item);

    if (raw_cmd == NULL)
    {
        return NULL;
    }

    return wintc_expand_desktop_entry_cmdline(
        raw_cmd,
        garcon_menu_item_get_name(item),
        garcon_menu_item_get_icon_name(item),
        garcon_menu_item_get_path(item),
        garcon_menu_item_requires_terminal(item)
    );
}
