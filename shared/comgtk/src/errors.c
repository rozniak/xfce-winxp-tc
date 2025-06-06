#include <glib.h>
#include <gtk/gtk.h>

#include "../public/errors.h"
#include "../public/msgbox.h"

#define RETURN_IF_NO_ERROR(e) if (e == NULL || *e == NULL) { return; }

//
// GLIB BOILERPLATE
//
G_DEFINE_QUARK(wintc-general-error-quark, wintc_general_error)

//
// PUBLIC FUNCTIONS
//
void wintc_display_error_and_clear(
    GError**   error,
    GtkWindow* wnd
)
{
    RETURN_IF_NO_ERROR(error)

    wintc_messagebox_show(
        wnd,
        (*error)->message,
        "",
        GTK_BUTTONS_OK,
        GTK_MESSAGE_ERROR
    );

    g_clear_error(error);
}

gboolean wintc_filter_error(
    GError*  error,
    GQuark   domain,
    gint     code,
    GError** out_error
)
{
    if (
        error->domain == domain &&
        error->code   == code
    )
    {
        g_propagate_error(out_error, error);
        return TRUE;
    }

    return FALSE;
}

void wintc_log_error_and_clear(
    GError** error
)
{
    RETURN_IF_NO_ERROR(error)

    g_message("%s", (*error)->message);

    g_clear_error(error);
}

void wintc_nice_error_and_clear(
    GError**   error,
    GtkWindow* wnd
)
{
    RETURN_IF_NO_ERROR(error)

    const gchar* message = NULL;

    if ((*error)->domain == WINTC_GENERAL_ERROR)
    {
        switch ((*error)->code)
        {
            case WINTC_GENERAL_ERROR_NOTIMPL:
                message = "Sorry, this feature is not implemented yet!";
                break;
        }
    }

    if (message != NULL)
    {
        wintc_messagebox_show(
            wnd,
            message,
            "",
            GTK_BUTTONS_OK,
            GTK_MESSAGE_ERROR
        );

        g_clear_error(error);
    }
    else
    {
        wintc_display_error_and_clear(error, wnd);
    }
}
