#ifndef __COMGTK_APPLICATION_H__
#define __COMGTK_APPLICATION_H__

#include <gio/gio.h>
#include <glib.h>

//
// PUBLIC FUNCTIONS
//
GFile** wintc_application_command_line_get_files(
    GApplicationCommandLine* command_line,
    guint*                   n_files
);

#endif
