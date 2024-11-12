#ifndef __SRCHSIDE_H__
#define __SRCHSIDE_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../toolbar.h"
#include "../window.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpSearchSidebarClass WinTCExpSearchSidebarClass;
typedef struct _WinTCExpSearchSidebar      WinTCExpSearchSidebar;

#define WINTC_TYPE_EXP_SEARCH_SIDEBAR            (wintc_exp_search_sidebar_get_type())
#define WINTC_EXP_SEARCH_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_SEARCH_SIDEBAR, WinTCExpSearchSidebar))
#define WINTC_EXP_SEARCH_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_SEARCH_SIDEBAR, WinTCExpSearchSidebarClass))
#define IS_WINTC_EXP_SEARCH_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_SEARCH_SIDEBAR))
#define IS_WINTC_EXP_SEARCH_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_SEARCH_SIDEBAR))
#define WINTC_EXP_SEARCH_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_SEARCH_SIDEBAR, WinTCExpSearchSidebar))

GType wintc_exp_search_sidebar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_search_sidebar_new(
    WinTCExplorerWindow* wnd
);

#endif
