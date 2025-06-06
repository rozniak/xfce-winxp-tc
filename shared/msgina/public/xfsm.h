#ifndef __GINA_XFSM_H__
#define __GINA_XFSM_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_GINA_SM_XFCE (wintc_gina_sm_xfce_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCGinaSmXfce,
    wintc_gina_sm_xfce,
    WINTC,
    GINA_SM_XFCE,
    GObject // FIXME: one day, implement a session management interface
)

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXfce* wintc_gina_sm_xfce_new(void);

gboolean wintc_gina_sm_xfce_is_valid(
    WinTCGinaSmXfce* sm_xfce
);

gboolean wintc_gina_sm_xfce_can_restart(
    WinTCGinaSmXfce* sm_xfce
);
gboolean wintc_gina_sm_xfce_can_shut_down(
    WinTCGinaSmXfce* sm_xfce
);
gboolean wintc_gina_sm_xfce_can_sleep(
    WinTCGinaSmXfce* sm_xfce
);

gboolean wintc_gina_sm_xfce_log_off(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
);
gboolean wintc_gina_sm_xfce_restart(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
);
gboolean wintc_gina_sm_xfce_shut_down(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
);
gboolean wintc_gina_sm_xfce_sleep(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
);
gboolean wintc_gina_sm_xfce_switch_user(
    WinTCGinaSmXfce* sm_xfce,
    GError**         error
);

#endif
