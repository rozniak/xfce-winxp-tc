#ifndef __SHEXT_ERROR_H__
#define __SHEXT_ERROR_H__

#include <glib.h>

#define WINTC_SHEXT_ERROR wintc_shext_error_quark()

typedef enum
{
    WINTC_SHEXT_ERROR_HOST_NO_VIEW = 1,
    WINTC_SHEXT_ERROR_HOST_TOPLEVEL_EXISTS,
    WINTC_SHEXT_ERROR_HOST_TOPLEVEL_NO_SUCH_CATEGORY
} WinTCShextError;

GQuark wintc_shext_error_quark(void);

#endif
