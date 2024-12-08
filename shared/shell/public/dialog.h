#ifndef __SHELL_DIALOG_H__
#define __SHELL_DIALOG_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
void wintc_sh_about(
    GtkWindow*   parent_wnd,
    const gchar* app_name,
    const gchar* other_stuff,
    const gchar* icon_name
);

#endif
