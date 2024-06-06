#ifndef __MSGINA_ERROR_H__
#define __MSGINA_ERROR_H__

#include <glib.h>

#define WINTC_GINA_ERROR wintc_gina_error_quark()

typedef enum
{
    WINTC_GINA_ERROR_NO_SESSIONS
} WinTCGinaError;

GQuark wintc_gina_error_quark(void);

#endif
