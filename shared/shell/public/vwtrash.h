#ifndef __SHELL_VWTRASH_H__
#define __SHELL_VWTRASH_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_VIEW_TRASH (wintc_sh_view_trash_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShViewTrash,
    wintc_sh_view_trash,
    WINTC,
    SH_VIEW_TRASH,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_trash_new(void);

#endif
