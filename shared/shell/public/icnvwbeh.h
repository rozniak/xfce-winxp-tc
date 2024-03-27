/** @file */

#ifndef __SHELL_ICNVWBEH_H__
#define __SHELL_ICNVWBEH_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "browser.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShIconViewBehaviourClass WinTCShIconViewBehaviourClass;
typedef struct _WinTCShIconViewBehaviour      WinTCShIconViewBehaviour;

#define WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR            (wintc_sh_icon_view_behaviour_get_type())
#define WINTC_SH_ICON_VIEW_BEHAVIOUR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR, WinTCShIconViewBehaviour))
#define WINTC_SH_ICON_VIEW_BEHAVIOUR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR, WinTCShIconViewBehaviourClass))
#define IS_WINTC_SH_ICON_VIEW_BEHAVIOUR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR))
#define IS_WINTC_SH_ICON_VIEW_BEHAVIOUR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR))
#define WINTC_SH_ICON_VIEW_BEHAVIOUR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_SH_ICON_VIEW_BEHAVIOUR, WinTCShIconViewBehaviour))

GType wintc_sh_icon_view_behaviour_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCShIconViewBehaviour* wintc_sh_icon_view_behaviour_new(
    GtkIconView*    icon_view,
    WinTCShBrowser* browser
);

#endif
