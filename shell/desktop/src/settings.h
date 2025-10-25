#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_DESKTOP_SETTINGS (wintc_desktop_settings_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCDesktopSettings,
    wintc_desktop_settings,
    WINTC,
    DESKTOP_SETTINGS,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCDesktopSettings* wintc_desktop_settings_new(void);

#endif
