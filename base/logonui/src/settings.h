#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include <glib.h>
#include <glib-object.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_LOGONUI_SETTINGS (wintc_logonui_settings_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCLogonUISettings,
    wintc_logonui_settings,
    WINTC,
    LOGONUI_SETTINGS,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCLogonUISettings* wintc_logonui_settings_new(void);

void wintc_logonui_settings_load_from_key_file(
    WinTCLogonUISettings* settings,
    GKeyFile*             key_file
);

const gchar* wintc_logonui_settings_get_session(
    WinTCLogonUISettings* settings
);
gboolean wintc_logonui_settings_get_use_classic_logon(
    WinTCLogonUISettings* settings
);
void wintc_logonui_settings_set_session(
    WinTCLogonUISettings* settings,
    const gchar*          session
);
void wintc_logonui_settings_set_use_classic_logon(
    WinTCLogonUISettings* settings,
    gboolean              use_classic_logon
);

#endif
