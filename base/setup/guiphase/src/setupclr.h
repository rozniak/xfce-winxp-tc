#ifndef __SETUPCLR_H__
#define __SETUPCLR_H__

#include <glib.h>

#include "setupwnd.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SETUP_CONTROLLER (wintc_setup_controller_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCSetupController,
    wintc_setup_controller,
    WINTC,
    SETUP_CONTROLLER,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCSetupController* wintc_setup_controller_new(
    WinTCSetupWindow* setup_wnd
);

void wintc_setup_controller_begin(
    WinTCSetupController* setup
);

#endif
