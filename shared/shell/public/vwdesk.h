#ifndef __SHELL_VWDESK_H__
#define __SHELL_VWDESK_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShViewDesktopClass WinTCShViewDesktopClass;
typedef struct _WinTCShViewDesktop      WinTCShViewDesktop;

#define WINTC_TYPE_SH_VIEW_DESKTOP            (wintc_sh_view_desktop_get_type())
#define WINTC_SH_VIEW_DESKTOP(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_VIEW_DESKTOP, WinTCShViewDesktop))
#define WINTC_SH_VIEW_DESKTOP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_VIEW_DESKTOP, WinTCShViewDesktopClass))
#define IS_WINTC_SH_VIEW_DESKTOP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_VIEW_DESKTOP))
#define IS_WINTC_SH_VIEW_DESKTOP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_VIEW_DESKTOP))
#define WINTC_SH_VIEW_DESKTOP_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_SH_VIEW_DESKTOP, WinTCShViewDesktop))

GType wintc_sh_view_desktop_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_desktop_new(void);

#endif
