#ifndef __SHELL_SOUND_H__
#define __SHELL_SOUND_H__

#include <glib.h>

//
// PUBLIC DEFINES
//
#define WINTC_SHELL_SND_ASTERISK "dialog-warning"
#define WINTC_SHELL_SND_BATTCRIT "battery-low"
#define WINTC_SHELL_SND_CRITSTOP "dialog-error"
#define WINTC_SHELL_SND_CLOSEPRG "windows-close-program"
#define WINTC_SHELL_SND_DEFAULT  "bell-window-system"
#define WINTC_SHELL_SND_DEVADD   "device-added-media"
#define WINTC_SHELL_SND_DEVREM   "device-removed-media"
#define WINTC_SHELL_SND_DEVFAIL  "windows-hardfail"
#define WINTC_SHELL_SND_EXCLAM   "dialog-question"
#define WINTC_SHELL_SND_EXITWIN  "desktop-logout"
#define WINTC_SHELL_SND_BATTLOW  "battery-caution"
#define WINTC_SHELL_SND_MAXIMIZE "windows-maximize"
#define WINTC_SHELL_SND_MENUCMD  "windows-menu-command"
#define WINTC_SHELL_SND_MENUPOP  "windows-menu-popup"
#define WINTC_SHELL_SND_MINIMIZE "windows-minimize"
#define WINTC_SHELL_SND_NEWMAIL  "message-new-email"
#define WINTC_SHELL_SND_OPENPRG  "windows-open-program"
#define WINTC_SHELL_SND_PRINTCMP "windows-print-complete"
#define WINTC_SHELL_SND_PROGERR  "windows-program-error"
#define WINTC_SHELL_SND_QUESTION "windows-question"
#define WINTC_SHELL_SND_RESTDOWN "windows-restore-down"
#define WINTC_SHELL_SND_RESTUP   "windows-restore-up"
#define WINTC_SHELL_SND_SELECT   "windows-select"
#define WINTC_SHELL_SND_SHOWTOOL "windows-show-toolbar-band"
#define WINTC_SHELL_SND_STARTWIN "desktop-login"
#define WINTC_SHELL_SND_SYSNOTIF "windows-system-notification"
#define WINTC_SHELL_SND_LOGOFF   "windows-logoff"
#define WINTC_SHELL_SND_LOGON    "windows-logon"

#define WINTC_SHELL_SND_BLKPOPUP "windows-blocked-popup"
#define WINTC_SHELL_SND_CMPNAV   "windows-complete-navigation"
#define WINTC_SHELL_SND_EMPTYBIN "trash-empty"
#define WINTC_SHELL_SND_INFOBAR  "windows-information-bar"
#define WINTC_SHELL_SND_MOVEITEM "windows-move-menu-item"
#define WINTC_SHELL_SND_STARTNAV "windows-start-navigation"

//
// PUBLIC FUNCTIONS
//
void wintc_sh_play_sound(
    const gchar* name
);

#endif
