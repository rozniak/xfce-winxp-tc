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
typedef struct _WinTCExplorerWindowClass WinTCExplorerWindowClass;
typedef struct _WinTCExplorerWindow      WinTCExplorerWindow;

#define WINTC_TYPE_EXPLORER_WINDOW            (wintc_explorer_window_get_type())
#define WINTC_EXPLORER_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindow))
#define WINTC_EXPLORER_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindowClass))
#define IS_WINTC_EXPLORER_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXPLORER_WINDOW))
#define IS_WINTC_EXPLORER_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXPLORER_WINDOW))
#define WINTC_EXPLORER_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindowClass))

GType wintc_explorer_window_get_type(void) G_GNUC_CONST;

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
void wintc_explorer_window_toggle_sidebar(
    WinTCExplorerWindow* wnd,
    const gchar*         sidebar_id
);

#endif
