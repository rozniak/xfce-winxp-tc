#include <glib.h>
#include <gtk/gtk.h>

#include "errors.h"
#include "msgbox.h"

//
// PUBLIC FUNCTIONS
//
void wintc_display_error_and_clear(
    GError** error
)
{
    wintc_messagebox_show(
        NULL,
        (*error)->message,
        "",
        GTK_BUTTONS_OK,
        GTK_MESSAGE_ERROR
    );

    g_clear_error(error);
}

void wintc_log_error_and_clear(
    GError** error
)
{
    g_message("%s", (*error)->message);

    g_clear_error(error);
}
