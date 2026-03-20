#ifndef __TOOLBAR_QACCESS_H__
#define __TOOLBAR_QACCESS_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_TOOLBAR_QUICK_ACCESS (wintc_toolbar_quick_access_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCToolbarQuickAccess,
    wintc_toolbar_quick_access,
    WINTC,
    TOOLBAR_QUICK_ACCESS,
    WinTCShextUIController
)

#endif
