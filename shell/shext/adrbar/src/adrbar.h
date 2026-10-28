#ifndef __ADRBAR_H__
#define __ADRBAR_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_ADDRESS (wintc_toolbar_address_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarAddress,
    wintc_toolbar_address,
    WINTC,
    TOOLBAR_ADDRESS,
    WinTCShextUIController
)

#endif
