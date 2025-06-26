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
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXfce* wintc_gina_sm_xfce_new(void);

#endif
