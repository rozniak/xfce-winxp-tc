/** @file */

#ifndef __EXEC_ERRORS_H__
#define __EXEC_ERRORS_H__

#include <glib.h>

#define WINTC_EXEC_ERROR wintc_exec_error_quark()

//
// PUBLIC ENUMS
//

/**
 * Specifies errors relevant to the launch APIs.
 */
typedef enum
{
    /** None of the fallbacks were available. */
    WINTC_EXEC_ERROR_FELLTHRU = 1,

    /** The desktop entry is malformed. */
    WINTC_EXEC_ERROR_BAD_DESKTOP_ENTRY = 2,
} WinTCExecError;

//
// PUBLIC FUNCTIONS
//
GQuark wintc_exec_error_quark(void);

#endif
