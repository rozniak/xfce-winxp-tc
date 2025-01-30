#include <gtk/gtk.h>

#include "../public/clipbd.h"

//
// PUBLIC FUNCTIONS
//
void wintc_clipboard_true_clear(
    GtkClipboard* clipboard
)
{
    gtk_clipboard_clear(clipboard);

    // If we don't own the clipboard, then replace it with nothing
    //
    gtk_clipboard_set_text(clipboard, "", 0);
}
