#ifndef __MSGBOX_H__
#define __MSGBOX_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//
gint wintc_messagebox_show(
    GtkWindow*     parent,
    const gchar*   text,
    const gchar*   caption,
    GtkButtonsType buttons,
    GtkMessageType type
);

#endif
