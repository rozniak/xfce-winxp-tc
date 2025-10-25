#ifndef __SHELL_FLDROPTS_H__
#define __SHELL_FLDROPTS_H__

#include <glib.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_FOLDER_OPTIONS (wintc_sh_folder_options_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShFolderOptions,
    wintc_sh_folder_options,
    WINTC,
    SH_FOLDER_OPTIONS,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShFolderOptions* wintc_sh_folder_options_new(void);

gboolean wintc_sh_folder_options_get_browse_in_same_window(
    WinTCShFolderOptions* fldr_opts
);
void wintc_sh_folder_options_set_browse_in_same_window(
    WinTCShFolderOptions* fldr_opts,
    gboolean              browse_in_same_window
);

#endif
