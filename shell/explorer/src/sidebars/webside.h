#ifndef __WEBSIDE_H__
#define __WEBSIDE_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../sidebar.h"
#include "../toolbar.h"
#include "../window.h"

//
// PUBLIC CONSTANTS
//
extern const gchar* WINTC_EXPLORER_SIDEBAR_ID_WEB;

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpWebSidebarClass WinTCExpWebSidebarClass;
typedef struct _WinTCExpWebSidebar      WinTCExpWebSidebar;

#define WINTC_TYPE_EXP_WEB_SIDEBAR            (wintc_exp_web_sidebar_get_type())
#define WINTC_EXP_WEB_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_WEB_SIDEBAR, WinTCExpWebSidebar))
#define WINTC_EXP_WEB_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_WEB_SIDEBAR, WinTCExpWebSidebarClass))
#define IS_WINTC_EXP_WEB_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_WEB_SIDEBAR))
#define IS_WINTC_EXP_WEB_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_WEB_SIDEBAR))
#define WINTC_EXP_WEB_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_WEB_SIDEBAR, WinTCExpWebSidebar))

GType wintc_exp_web_sidebar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_web_sidebar_new(
    WinTCExplorerWindow* wnd
);

#endif
