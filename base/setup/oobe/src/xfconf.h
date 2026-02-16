#ifndef __XFCONF_H__
#define __XFCONF_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//
void wintc_oobe_xfconf_update_channel(
    const gchar* user_home,
    const gchar* channel
);

#endif
