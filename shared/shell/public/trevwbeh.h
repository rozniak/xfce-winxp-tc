/** @file */

#ifndef __SHELL_TREVWBEH_H__
#define __SHELL_TREVWBEH_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "browser.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShTreeViewBehaviourClass WinTCShTreeViewBehaviourClass;
typedef struct _WinTCShTreeViewBehaviour      WinTCShTreeViewBehaviour;

#define WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR (wintc_sh_tree_view_behaviour_get_type())
#define WINTC_SH_TREE_VIEW_BEHAVIOUR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR, WinTCShTreeViewBehaviour))
#define WINTC_SH_TREE_VIEW_BEHAVIOUR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR, WinTCShTreeViewBehaviourClass))
#define IS_WINTC_SH_TREE_VIEW_BEHAVIOUR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR))
#define IS_WINTC_SH_TREE_VIEW_BEHAVIOUR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR))
#define WINTC_SH_TREE_VIEW_BEHAVIOUR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SH_TREE_VIEW_BEHAVIOUR, WinTCShTreeViewBehaviour))

GType wintc_sh_tree_view_behaviour_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCShTreeViewBehaviour* wintc_sh_tree_view_behaviour_new(
    GtkTreeView*    icon_view,
    WinTCShBrowser* browser
);

#endif
