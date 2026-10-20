#ifndef __ADRBAR_H__
#define __ADRBAR_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_EXP_ADDRESS_TOOLBAR (wintc_exp_address_toolbar_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCExpAddressToolbar,
    wintc_exp_address_toolbar,
    WINTC,
    EXP_ADDRESS_TOOLBAR,
    WinTCShextUIController
)

#endif
