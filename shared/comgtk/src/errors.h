#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <glib.h>

#define WINTC_GENERAL_ERROR wintc_general_error_quark()

typedef enum
{
    WINTC_GENERAL_ERROR_NOTIMPL
} WinTCGeneralError;

void wintc_display_error_and_clear(
    GError** error
);

GQuark wintc_general_error_quark(void);

void wintc_log_error_and_clear(
    GError** error
);

void wintc_nice_error_and_clear(
    GError** error
);

#endif
