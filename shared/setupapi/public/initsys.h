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

typedef enum {
    // Reserved for future API expansion
    WINTC_INITSYS_PRIORITY_BEFORE_DM = 10
} WinTCInitSystemPriority;

//
// PUBLIC FUNCTIONS
//
WinTCInitSystem wintc_init_system_get(void);

const gchar* wintc_init_system_get_name(
    WinTCInitSystem init_sys
);

gboolean wintc_init_system_disable_service(
    const gchar* service_name,
    GError**     error
);
gboolean wintc_init_system_enable_service(
    const gchar*            service_name,
    WinTCInitSystemPriority priority,
    GError**                error
);

#endif
