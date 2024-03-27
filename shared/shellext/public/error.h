#ifndef __SHEXT_ERROR_H__
#define __SHEXT_ERROR_H__

#include <glib.h>

#define WINTC_SHEXT_ERROR wintc_shext_error_quark()

typedef enum
{
    WINTC_SHEXT_ERROR_HOST_NO_VIEW
} WinTCShextError;

GQuark wintc_shext_error_quark(void);

#endif
