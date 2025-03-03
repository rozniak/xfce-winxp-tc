#ifndef __COMCTL_MENUBIND_H__
#define __COMCTL_MENUBIND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CTL_MENU_BINDING (wintc_ctl_menu_binding_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCCtlMenuBinding,
    wintc_ctl_menu_binding,
    WINTC,
    CTL_MENU_BINDING,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCCtlMenuBinding* wintc_ctl_menu_binding_new(
    GtkMenuShell* menu_shell,
    GMenuModel*   menu_model
);

#endif
