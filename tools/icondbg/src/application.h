#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_ICON_DBG_APPLICATION (wintc_icon_dbg_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCIconDbgApplication,
    wintc_icon_dbg_application,
    WINTC,
    ICON_DBG_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCIconDbgApplication* wintc_icon_dbg_application_new(void);

#endif
