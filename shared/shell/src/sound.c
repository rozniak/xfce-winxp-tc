#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/sound.h"

//
// STATIC DATA
//
static const gchar* S_XDG_SOUNDS[] = {
    "dialog-warning",
    "battery-low",
    "dialog-error",
    "windows-close-program",
    "bell-window-system",
    "device-added-media",
    "device-removed-media",
    "windows-hardfail",
    "dialog-question",
    "desktop-logout",
    "battery-caution",
    "windows-maximize",
    "windows-menu-command",
    "windows-menu-popup",
    "windows-minimize",
    "message-new-email",
    "windows-open-program",
    "windows-print-complete",
    "windows-program-error",
    "windows-question",
    "windows-restore-down",
    "windows-restore-up",
    "windows-select",
    "windows-show-toolbar-band",
    "desktop-login",
    "windows-system-notification",
    "windows-logoff",
    "windows-logon",

    "windows-blocked-popup",
    "windows-complete-navigation",
    "trash-empty",
    "windows-information-bar",
    "windows-move-menu-item",
    "windows-start-navigation"
};

//
// PUBLIC FUNCTIONS
//
void wintc_sh_play_sound(
    WinTCShSound sound
)
{
    wintc_xdg_play_sound(S_XDG_SOUNDS[sound]);
}
