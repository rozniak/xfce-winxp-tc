#ifndef __SETUPAPI_ERROR_H__
#define __SETUPAPI_ERROR_H__

#include <glib.h>

#define WINTC_SETUPAPI_ERROR wintc_setupapi_error_quark()

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SETUPAPI_ERROR_FAILED /** Package process failed. */
} WinTCSetupApiError;

//
// PUBLIC FUNCTIONS
//
GQuark wintc_setupapi_error_quark(void);

#endif
