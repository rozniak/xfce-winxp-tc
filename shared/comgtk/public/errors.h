/** @file */

#ifndef __COMGTK_ERRORS_H__
#define __COMGTK_ERRORS_H__

#include <glib.h>
#include <gtk/gtk.h>

#define WINTC_GENERAL_ERROR wintc_general_error_quark()

#define WINTC_SUCCESS(expr, err, success)  \
    (expr);                                \
    if ((err))                             \
    {                                      \
        wintc_log_error_and_clear(&(err)); \
        success = FALSE;                   \
    }

/**
 * Specifies simple errors used by WinTC components.
 */
typedef enum
{
    WINTC_GENERAL_ERROR_NOTIMPL /** The functionality is not implemented. */
} WinTCGeneralError;

/**
 * Displays a message box describing the error and clears the associated
 * GError object.
 *
 * @param error A reference to the GError storage location.
 * @param wnd   The window to own the dialog, if applicable.
 */
void wintc_display_error_and_clear(
    GError**   error,
    GtkWindow* wnd
);

GQuark wintc_general_error_quark(void);

/**
 * Logs a message describing the error and clears the associated GError
 * object.
 *
 * @param error A reference to the GError storage location.
 */
void wintc_log_error_and_clear(
    GError** error
);

/**
 * Displays a message box describing the error in a nicer way, if possible, and
 * clears the associated GError object.
 *
 * @param error A reference to the GError storage location.
 * @param wnd   The window to own the dialog, if applicable.
 */
void wintc_nice_error_and_clear(
    GError**   error,
    GtkWindow* wnd
);

#endif
