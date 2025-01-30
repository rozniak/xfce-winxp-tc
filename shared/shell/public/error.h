#ifndef __SHELL_ERROR_H__
#define __SHELL_ERROR_H__

#include <glib.h>

#define WINTC_SHELL_ERROR wintc_shell_error_quark()

typedef enum
{
    WINTC_SHELL_ERROR_CLIPBOARD_EMPTY,
    WINTC_SHELL_ERROR_CLIPBOARD_GET_FAILED
} WinTCShellError;

GQuark wintc_shell_error_quark(void);

#endif
