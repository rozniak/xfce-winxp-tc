#ifndef __SHELL_FSCLIPBD_H__
#define __SHELL_FSCLIPBD_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_FS_CLIPBOARD (wintc_sh_fs_clipboard_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShFSClipboard,
    wintc_sh_fs_clipboard,
    WINTC,
    SH_FS_CLIPBOARD,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShFSClipboard* wintc_sh_fs_clipboard_new(void);

gboolean wintc_sh_fs_clipboard_paste(
    WinTCShFSClipboard* fs_clipboard,
    const gchar*        dest,
    GtkWindow*          wnd,
    GError**            error
);

#endif
