#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_CPL_APPWIZ_APPLICATION (wintc_cpl_appwiz_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCCplAppwizApplication,
    wintc_cpl_appwiz_application,
    WINTC,
    CPL_APPWIZ_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCCplAppwizApplication* wintc_cpl_appwiz_application_new(void);

#endif
