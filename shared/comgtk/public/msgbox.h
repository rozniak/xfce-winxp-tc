/** @file */

#ifndef __COMGTK_MSGBOX_H__
#define __COMGTK_MSGBOX_H__

#include <gtk/gtk.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Displays a message box with specified options.
 *
 * @param parent  The parent window of the dialog.
 * @param text    The message to display.
 * @param caption The title of the dialog.
 * @param buttons The available buttons.
 * @param type    The type of message being displayed.
 * @return The choice the user made in the dialog.
 */
gint wintc_messagebox_show(
    GtkWindow*     parent,
    const gchar*   text,
    const gchar*   caption,
    GtkButtonsType buttons,
    GtkMessageType type
);

#endif
