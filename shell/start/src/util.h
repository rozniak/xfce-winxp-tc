#ifndef __UTIL_H__
#define __UTIL_H__

#include <garcon/garcon.h>
#include <glib.h>

gchar* garcon_menu_item_get_command_expanded(
    GarconMenuItem* item
);

#endif
