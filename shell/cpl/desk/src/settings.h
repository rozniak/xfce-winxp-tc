#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>
#include <wintc/syscfg.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplDeskSettingsClass WinTCCplDeskSettingsClass;
typedef struct _WinTCCplDeskSettings      WinTCCplDeskSettings;

#define WINTC_TYPE_CPL_DESK_SETTINGS            (wintc_cpl_desk_settings_get_type())
#define WINTC_CPL_DESK_SETTINGS(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_DESK_SETTINGS, WinTCCplDeskSettings))
#define WINTC_CPL_DESK_SETTINGS_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_DESK_SETTINGS, WinTCCplDeskSettingsClass))
#define IS_WINTC_CPL_DESK_SETTINGS(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_DESK_SETTINGS))
#define IS_WINTC_CPL_DESK_SETTINGS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_DESK_SETTINGS))
#define WINTC_CPL_DESK_SETTINGS_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CPL_DESK_SETTINGS, WinTCCplDeskSettingsClass))

GType wintc_cpl_desk_settings_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCCplDeskSettings* wintc_cpl_desk_settings_new(void);

gboolean wintc_cpl_desk_settings_apply(
    WinTCCplDeskSettings* settings,
    GError**              error
);
gboolean wintc_cpl_desk_settings_load(
    WinTCCplDeskSettings* settings,
    GError**              error
);

const gchar* wintc_cpl_desk_settings_get_wallpaper(
    WinTCCplDeskSettings* settings
);
WinTCWallpaperStyle wintc_cpl_desk_settings_get_wallpaper_style(
    WinTCCplDeskSettings* settings
);
void wintc_cpl_desk_settings_set_wallpaper(
    WinTCCplDeskSettings* settings,
    const gchar*          path
);
void wintc_cpl_desk_settings_set_wallpaper_style(
    WinTCCplDeskSettings* settings,
    WinTCWallpaperStyle   style
);

#endif
