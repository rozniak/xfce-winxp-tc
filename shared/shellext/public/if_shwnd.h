/** @file */

#ifndef __SHELLEXT_IF_SHWND_H__
#define __SHELLEXT_IF_SHWND_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "if_view.h"

//
// PUBLIC ENUMS
//
typedef enum _WinTCShextBrowserMode
{
    WINTC_SHEXT_BROWSER_MODE_INVALID = 0,
    WINTC_SHEXT_BROWSER_MODE_LOCAL,
    WINTC_SHEXT_BROWSER_MODE_INTERNET
} WinTCShextBrowserMode;

typedef enum _WinTCShextShellWindowExtension
{
    WINTC_SHEXT_SHWND_HOSTEXT_INVALID = 0,
    WINTC_SHEXT_SHWND_HOSTEXT_TOOLBAR
} WinTCShextShellWindowExtension;

typedef enum _WinTCShextShellWindowKind
{
    WINTC_SHEXT_SHWND_KIND_INVALID = 0,
    WINTC_SHEXT_SHWND_KIND_BROWSER,
    WINTC_SHEXT_SHWND_KIND_CHROME
} WinTCShextShellWindowKind;

//
// GTK INTERFACE DEFINITIONS
//
#define WINTC_TYPE_ISHEXT_SHELL_WINDOW (wintc_ishext_shell_window_get_type())

G_DECLARE_INTERFACE(
    WinTCIShextShellWindow,
    wintc_ishext_shell_window,
    WINTC,
    ISHEXT_SHELL_WINDOW,
    GObject
)

struct _WinTCIShextShellWindowInterface
{
    GTypeInterface base_iface;

    void (*get_location) (
        WinTCIShextShellWindow* shwnd,
        WinTCShextPathInfo*     path_info
    );
    WinTCShextBrowserMode (*get_mode) (
        WinTCIShextShellWindow* shwnd
    );
    gboolean (*navigate) (
        WinTCIShextShellWindow*   shwnd,
        const WinTCShextPathInfo* path_info,
        GError**                  error
    );
    gboolean (*navigate_simple) (
        WinTCIShextShellWindow* shwnd,
        const gchar*            path,
        GError**                error
    );
};

//
// INTERFACE METHODS
//
void wintc_ishext_shell_window_get_location(
    WinTCIShextShellWindow* shwnd,
    WinTCShextPathInfo*     path_info
);
WinTCShextBrowserMode wintc_ishext_shell_window_get_mode(
    WinTCIShextShellWindow* shwnd
);

gboolean wintc_ishext_shell_window_navigate(
    WinTCIShextShellWindow*   shwnd,
    const WinTCShextPathInfo* path_info,
    GError**                  error
);
gboolean wintc_ishext_shell_window_navigate_simple(
    WinTCIShextShellWindow* shwnd,
    const gchar*            path,
    GError**                error
);

//
// PUBLIC FUNCTIONS
//
void _wintc_ishext_shell_window_location_changed(
    WinTCIShextShellWindow* shwnd
);
void _wintc_ishext_shell_window_mode_changed(
    WinTCIShextShellWindow* shwnd
);

#endif
