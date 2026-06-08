/** @file */

#ifndef __COMGTK_MSGBOX_H__
#define __COMGTK_MSGBOX_H__

#include <gtk/gtk.h>

//
// PUBLIC ENUMS
//
typedef enum _WinTCButtonsType
{
    WINTC_BUTTONS_OK,
    WINTC_BUTTONS_OK_CANCEL,
    WINTC_BUTTONS_ABORT_RETRY_IGNORE,
    WINTC_BUTTONS_YES_NO_CANCEL,
    WINTC_BUTTONS_YES_NO,
    WINTC_BUTTONS_RETRY_CANCEL,
    WINTC_BUTTONS_CANCEL_TRY_CONTINUE
} WinTCButtonsType;

typedef enum _WinTCMessageType
{
    WINTC_MESSAGE_NONE,
    WINTC_MESSAGE_ERROR,
    WINTC_MESSAGE_QUESTION,
    WINTC_MESSAGE_WARNING,
    WINTC_MESSAGE_INFORMATION
} WinTCMessageType;

typedef enum _WinTCResponseType
{
    WINTC_RESPONSE_NONE,
    WINTC_RESPONSE_OK,
    WINTC_RESPONSE_CANCEL,
    WINTC_RESPONSE_YES,
    WINTC_RESPONSE_NO,
    WINTC_RESPONSE_ABORT,
    WINTC_RESPONSE_RETRY,
    WINTC_RESPONSE_IGNORE,
    WINTC_RESPONSE_TRY_AGAIN,
    WINTC_RESPONSE_CONTINUE
} WinTCResponseType;

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
WinTCResponseType wintc_messagebox_show(
    GtkWindow*       parent,
    const gchar*     text,
    const gchar*     caption,
    WinTCButtonsType buttons,
    WinTCMessageType type
);

#endif
