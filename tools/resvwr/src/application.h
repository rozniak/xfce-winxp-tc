#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_RESVWR_APPLICATION (wintc_resvwr_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCResvwrApplication,
    wintc_resvwr_application,
    WINTC,
    RESVWR_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCResvwrApplication* wintc_resvwr_application_new(void);

#endif
