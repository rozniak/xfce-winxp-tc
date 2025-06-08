#ifndef __MSGINA_IF_SM_H__
#define __MSGINA_IF_SM_H__

#include <glib.h>

//
// GTK INTERFACE DEFINITIONS
//
#define WINTC_TYPE_IGINA_SM (wintc_igina_sm_get_type())

G_DECLARE_INTERFACE(
    WinTCIGinaSm,
    wintc_igina_sm,
    WINTC,
    IGINA_SM,
    GObject
)

struct _WinTCIGinaSmInterface
{
    GTypeInterface base_iface;

    gboolean (*is_valid) (
        WinTCIGinaSm* sm
    );

    gboolean (*can_restart) (
        WinTCIGinaSm* sm
    );
    gboolean (*can_shut_down) (
        WinTCIGinaSm* sm
    );
    gboolean (*can_sleep) (
        WinTCIGinaSm* sm
    );

    gboolean (*log_off) (
        WinTCIGinaSm* sm,
        GError**      error
    );
    gboolean (*restart) (
        WinTCIGinaSm* sm,
        GError**      error
    );
    gboolean (*shut_down) (
        WinTCIGinaSm* sm,
        GError**      error
    );
    gboolean (*sleep) (
        WinTCIGinaSm* sm,
        GError**      error
    );
    gboolean (*switch_user) (
        WinTCIGinaSm* sm,
        GError**      error
    );
};

//
// INTERFACE METHODS
//
gboolean wintc_igina_sm_is_valid(
    WinTCIGinaSm* sm
);

gboolean wintc_igina_sm_can_restart(
    WinTCIGinaSm* sm
);
gboolean wintc_igina_sm_can_shut_down(
    WinTCIGinaSm* sm
);
gboolean wintc_igina_sm_can_sleep(
    WinTCIGinaSm* sm
);

gboolean wintc_igina_sm_log_off(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_igina_sm_restart(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_igina_sm_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_igina_sm_sleep(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_igina_sm_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
);

#endif
