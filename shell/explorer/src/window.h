#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>

#include "application.h"
#include "loader.h"

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_EXPLORER_WINDOW_MODE_INVALID = 0,
    WINTC_EXPLORER_WINDOW_MODE_LOCAL,
    WINTC_EXPLORER_WINDOW_MODE_INTERNET
} WinTCExplorerWindowMode;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_EXPLORER_WINDOW (wintc_explorer_window_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCExplorerWindow,
    wintc_explorer_window,
    WINTC,
    EXPLORER_WINDOW,
    GtkApplicationWindow
)

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_explorer_window_new(
    WinTCExplorerApplication* app,
    WinTCShextHost*           shext_host,
    WinTCShFolderOptions*     fldr_opts,
    WinTCExplorerLoader*      loader,
    const gchar*              initial_path
);

WinTCShBrowser* wintc_explorer_window_get_browser(
    WinTCExplorerWindow* wnd
);
void wintc_explorer_window_get_location(
    WinTCExplorerWindow* wnd,
    WinTCShextPathInfo*  path_info
);
WinTCExplorerWindowMode wintc_explorer_window_get_mode(
    WinTCExplorerWindow* wnd
);
GList* wintc_explorer_window_get_selected_items(
    WinTCExplorerWindow* wnd
);
void wintc_explorer_window_toggle_sidebar(
    WinTCExplorerWindow* wnd,
    const gchar*         sidebar_id
);

#endif
