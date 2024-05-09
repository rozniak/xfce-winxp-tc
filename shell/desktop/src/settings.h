#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDesktopSettingsClass WinTCDesktopSettingsClass;
typedef struct _WinTCDesktopSettings      WinTCDesktopSettings;

#define WINTC_TYPE_DESKTOP_SETTINGS            (wintc_desktop_settings_get_type())
#define WINTC_DESKTOP_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_DESKTOP_SETTINGS, WinTCDesktopSettings))
#define WINTC_DESKTOP_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_DESKTOP_SETTINGS, WinTCDesktopSettingsClass))
#define IS_WINTC_DESKTOP_SETTINGS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_DESKTOP_SETTINGS))
#define IS_WINTC_DESKTOP_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_DESKTOP_SETTINGS))
#define WINTC_DESKTOP_SETTINGS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_DESKTOP_SETTINGS, WinTCDesktopSettingsClass))

GType wintc_desktop_settings_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCDesktopSettings* wintc_desktop_settings_new(void);

#endif
