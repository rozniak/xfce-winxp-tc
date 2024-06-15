/** @file */

#ifndef __SHELL_BROWSER_H__
#define __SHELL_BROWSER_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_SH_BROWSER_LOAD_STARTED = 0,
    WINTC_SH_BROWSER_LOAD_FINISHED
} WinTCShBrowserLoadEvent;

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShBrowserClass WinTCShBrowserClass;

/**
 * Provides the main browser behaviour.
 */
typedef struct _WinTCShBrowser WinTCShBrowser;

#define WINTC_TYPE_SH_BROWSER            (wintc_sh_browser_get_type())
#define WINTC_SH_BROWSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_BROWSER, WinTCShBrowser))
#define WINTC_SH_BROWSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_BROWSER, WinTCShBrowserClass))
#define IS_WINTC_SH_BROWSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_BROWSER))
#define IS_WINTC_SH_BROWSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_BROWSER))
#define WINTC_SH_BROWSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SH_BROWSER, WinTCShBrowser))

GType wintc_sh_browser_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCShBrowser* wintc_sh_browser_new(
    WinTCShextHost* shext_host
);

gboolean wintc_sh_browser_activate_item(
    WinTCShBrowser*     browser,
    WinTCShextViewItem* item,
    GError**            error
);

gboolean wintc_sh_browser_can_navigate_to_parent(
    WinTCShBrowser* browser
);

void wintc_sh_browser_get_location(
    WinTCShBrowser*     browser,
    WinTCShextPathInfo* path_info
);

GtkTreeModel* wintc_sh_browser_get_model(
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

gboolean wintc_sh_browser_set_location(
    WinTCShBrowser*           browser,
    const WinTCShextPathInfo* path_info,
    GError**                  error
);

#endif
