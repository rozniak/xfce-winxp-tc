#ifndef __SETUPAPI_INITSYS_H__
#define __SETUPAPI_INITSYS_H__

#include <glib.h>

//
// PUBLIC ENUMS
//
typedef enum {
    WINTC_INITSYS_UNKNOWN,
    WINTC_INITSYS_SYSTEMD,
    WINTC_INITSYS_RUNIT,
    WINTC_INITSYS_UPSTART,
    WINTC_INITSYS_SYSVINIT,
    WINTC_INITSYS_OPENRC
} WinTCInitSystem;

//
// PUBLIC FUNCTIONS
//
WinTCInitSystem wintc_get_init_system(void);

const gchar* wintc_get_init_system_name(
    WinTCInitSystem init_sys
);

#endif
