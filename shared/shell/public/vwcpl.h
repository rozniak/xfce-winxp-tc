#ifndef __SHELL_VWCPL_H__
#define __SHELL_VWCPL_H__

#include <glib.h>
#include <wintc/shellext.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCShViewCplClass WinTCShViewCplClass;
typedef struct _WinTCShViewCpl      WinTCShViewCpl;

#define WINTC_TYPE_SH_VIEW_CPL            (wintc_sh_view_cpl_get_type())
#define WINTC_SH_VIEW_CPL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_SH_VIEW_CPL, WinTCShViewCpl))
#define WINTC_SH_VIEW_CPL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_SH_VIEW_CPL, WinTCShViewCplClass))
#define IS_WINTC_SH_VIEW_CPL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_SH_VIEW_CPL))
#define IS_WINTC_SH_VIEW_CPL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_SH_VIEW_CPL))
#define WINTC_SH_VIEW_CPL_GET_CLASS(obj)  (G_TYPE_INSANCE_GET_CLASS((obj), WINTC_TYPE_SH_VIEW_CPL, WinTCShViewCpl))

GType wintc_sh_view_cpl_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCIShextView* wintc_sh_view_cpl_new(void);

#endif
