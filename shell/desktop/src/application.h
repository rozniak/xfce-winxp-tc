#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_DESKTOP_APPLICATION (wintc_desktop_application_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCDesktopApplication,
    wintc_desktop_application,
    WINTC,
    DESKTOP_APPLICATION,
    GtkApplication
)

//
// PUBLIC FUNCTIONS
//
WinTCDesktopApplication* wintc_desktop_application_new(void);

#endif
