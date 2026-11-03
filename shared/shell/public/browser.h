/** @file */

#ifndef __SHELL_BROWSER_H__
#define __SHELL_BROWSER_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

#include "fldropts.h"

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SH_BROWSER_LOAD_STARTED = 0,
    WINTC_SH_BROWSER_LOAD_FINISHED
} WinTCShBrowserLoadEvent;

typedef enum
{
    WINTC_SH_BROWSER_BEHAVIOUR_DONT_USE_SELF_EXE = 0x1
} WinTCShBrowserBehaviourFlag;

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_SH_BROWSER (wintc_sh_browser_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCShBrowser,
    wintc_sh_browser,
    WINTC,
    SH_BROWSER,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCShBrowser* wintc_sh_browser_new(
    WinTCShextHost*       shext_host,
    WinTCShFolderOptions* fldr_opts
);

gboolean wintc_sh_browser_activate_item(
    WinTCShBrowser*     browser,
    WinTCIShextView*    view,
    guint               item_hash,
    GError**            error
);

gboolean wintc_sh_browser_can_navigate_to_parent(
    WinTCShBrowser* browser
);

WinTCIShextView* wintc_sh_browser_get_current_view(
    WinTCShBrowser* browser
);

WinTCShBrowserBehaviourFlag wintc_sh_browser_get_behaviour_flags(
    WinTCShBrowser* browser
);

void wintc_sh_browser_get_location(
    WinTCShBrowser*     browser,
    WinTCShextPathInfo* path_info
);

WinTCShextHost* wintc_sh_browser_get_shext_host(
    WinTCShBrowser* browser
);

const gchar* wintc_sh_browser_get_view_display_name(
    WinTCShBrowser* browser
);

void wintc_sh_browser_navigate_to_parent(
    WinTCShBrowser* browser
);

void wintc_sh_browser_refresh(
    WinTCShBrowser* browser
);

void wintc_sh_browser_set_behaviour_flags(
    WinTCShBrowser*             browser,
    WinTCShBrowserBehaviourFlag flags
);

gboolean wintc_sh_browser_set_location(
    WinTCShBrowser*           browser,
    const WinTCShextPathInfo* path_info,
    GError**                  error
);

#endif
