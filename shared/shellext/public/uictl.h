#ifndef __SHELLEXT_UICTL_H__
#define __SHELLEXT_UICTL_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "if_uihost.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SHEXT_UI_CONTROLLER (wintc_shext_ui_controller_get_type())

G_DECLARE_DERIVABLE_TYPE(
    WinTCShextUIController,
    wintc_shext_ui_controller,
    WINTC,
    SHEXT_UI_CONTROLLER,
    GObject
)

struct _WinTCShextUIControllerClass
{
    GObjectClass __parent__;
};

//
// PUBLIC FUNCTIONS
//
WinTCShextUIController* wintc_shext_ui_controller_new_from_type(
    GType              type,
    WinTCIShextUIHost* ui_host
);

WinTCIShextUIHost* wintc_shext_ui_controller_get_ui_host(
    WinTCShextUIController* ui_ctl
);

#endif
