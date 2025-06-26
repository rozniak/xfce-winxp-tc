#ifndef __GINA_XDGSM_H__
#define __GINA_XDGSM_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_GINA_SM_XDG (wintc_gina_sm_xdg_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCGinaSmXdg,
    wintc_gina_sm_xdg,
    WINTC,
    GINA_SM_XDG,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXdg* wintc_gina_sm_xdg_new(void);

#endif
