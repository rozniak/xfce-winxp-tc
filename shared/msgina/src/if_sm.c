#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_sm.h"

//
// GTK INTERFACE DEFINITIONS & CTORS
//
G_DEFINE_INTERFACE(
    WinTCIGinaSm,
    wintc_igina_sm,
    G_TYPE_OBJECT
)

static void wintc_igina_sm_default_init(
    WINTC_UNUSED(WinTCIGinaSmInterface* iface)
) {}

//
// INTERFACE METHODS
//
gboolean wintc_igina_sm_is_valid(
    WinTCIGinaSm* sm
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->is_valid(sm);
}

gboolean wintc_igina_sm_can_restart(
    WinTCIGinaSm* sm
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->can_restart(sm);
}

gboolean wintc_igina_sm_can_shut_down(
    WinTCIGinaSm* sm
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->can_shut_down(sm);
}

gboolean wintc_igina_sm_can_sleep(
    WinTCIGinaSm* sm
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->can_sleep(sm);
}

gboolean wintc_igina_sm_log_off(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->log_off(sm, error);
}

gboolean wintc_igina_sm_restart(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->restart(sm, error);
}

gboolean wintc_igina_sm_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->shut_down(sm, error);
}

gboolean wintc_igina_sm_sleep(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->sleep(sm, error);
}

gboolean wintc_igina_sm_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCIGinaSmInterface* iface =
        WINTC_IGINA_SM_GET_IFACE(sm);

    return iface->switch_user(sm, error);
}
