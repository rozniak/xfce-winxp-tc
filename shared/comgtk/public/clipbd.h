/** @file */

#ifndef __COMGTK_CLIPBD_H__
#define __COMGTK_CLIPBD_H__

#include <glib.h>

//
// PUBLIC FUNCTIONS
//

/**
 * Clears the contents of the clipboard regardless of whether the clipboard
 * is owned by someone else or not.
 *
 * @param clipboard The clipboard.
 */
void wintc_clipboard_true_clear(
    GtkClipboard* clipboard
);

#endif
