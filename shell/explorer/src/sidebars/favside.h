#ifndef __FAVSIDE_H__
#define __FAVSIDE_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../sidebar.h"
#include "../toolbar.h"
#include "../window.h"

//
// PUBLIC CONSTANTS
//
extern const gchar* WINTC_EXPLORER_SIDEBAR_ID_FAVORITES;

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpFavoritesSidebarClass WinTCExpFavoritesSidebarClass;
typedef struct _WinTCExpFavoritesSidebar      WinTCExpFavoritesSidebar;

#define WINTC_TYPE_EXP_FAVORITES_SIDEBAR            (wintc_exp_favorites_sidebar_get_type())
#define WINTC_EXP_FAVORITES_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_FAVORITES_SIDEBAR, WinTCExpFavoritesSidebar))
#define WINTC_EXP_FAVORITES_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_FAVORITES_SIDEBAR, WinTCExpFavoritesSidebarClass))
#define IS_WINTC_EXP_FAVORITES_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_FAVORITES_SIDEBAR))
#define IS_WINTC_EXP_FAVORITES_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_FAVORITES_SIDEBAR))
#define WINTC_EXP_FAVORITES_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_FAVORITES_SIDEBAR, WinTCExpFavoritesSidebar))

GType wintc_exp_favorites_sidebar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_favorites_sidebar_new(
    WinTCExplorerWindow* wnd
);

#endif
