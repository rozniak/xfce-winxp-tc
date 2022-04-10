#ifndef __EXEC_H__
#define __EXEC_H__

#include <glib.h>

gboolean wintc_launch_command(
    const gchar* command,
    GError**     out_error
);

#endif
