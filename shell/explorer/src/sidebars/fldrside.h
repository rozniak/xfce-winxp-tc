#ifndef __FLDRSIDE_H__
#define __FLDRSIDE_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../sidebar.h"
#include "../toolbar.h"
#include "../window.h"

//
// PUBLIC CONSTANTS
//
extern const gchar* WINTC_EXPLORER_SIDEBAR_ID_FOLDERS;

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpFoldersSidebarClass WinTCExpFoldersSidebarClass;
typedef struct _WinTCExpFoldersSidebar      WinTCExpFoldersSidebar;

#define WINTC_TYPE_EXP_FOLDERS_SIDEBAR            (wintc_exp_folders_sidebar_get_type())
#define WINTC_EXP_FOLDERS_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_FOLDERS_SIDEBAR, WinTCExpFoldersSidebar))
#define WINTC_EXP_FOLDERS_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_FOLDERS_SIDEBAR, WinTCExpFoldersSidebarClass))
#define IS_WINTC_EXP_FOLDERS_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_FOLDERS_SIDEBAR))
#define IS_WINTC_EXP_FOLDERS_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_FOLDERS_SIDEBAR))
#define WINTC_EXP_FOLDERS_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_FOLDERS_SIDEBAR, WinTCExpFoldersSidebar))

GType wintc_exp_folders_sidebar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerSidebar* wintc_exp_folders_sidebar_new(
    WinTCExplorerWindow* wnd
);

#endif
