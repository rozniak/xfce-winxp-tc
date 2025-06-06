#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_EXITWIN_APPLICATION (wintc_exitwin_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCExitwinApplication,
    wintc_exitwin_application,
    WINTC,
    EXITWIN_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCExitwinApplication* wintc_exitwin_application_new(void);

#endif
