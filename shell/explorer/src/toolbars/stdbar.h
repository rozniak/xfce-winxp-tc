#ifndef __STDBAR_H__
#define __STDBAR_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../toolbar.h"
#include "../window.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpStandardToolbarClass WinTCExpStandardToolbarClass;
typedef struct _WinTCExpStandardToolbar      WinTCExpStandardToolbar;

#define WINTC_TYPE_EXP_STANDARD_TOOLBAR            (wintc_exp_standard_toolbar_get_type())
#define WINTC_EXP_STANDARD_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_STANDARD_TOOLBAR, WinTCExpStandardToolbar))
#define WINTC_EXP_STANDARD_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_STANDARD_TOOLBAR, WinTCExpStandardToolbarClass))
#define IS_WINTC_EXP_STANDARD_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_STANDARD_TOOLBAR))
#define IS_WINTC_EXP_STANDARD_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_STANDARD_TOOLBAR))
#define WINTC_EXP_STANDARD_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_STANDARD_TOOLBAR, WinTCExpStandardToolbar))

GType wintc_exp_standard_toolbar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerToolbar* wintc_exp_standard_toolbar_new(
    WinTCExplorerWindow* wnd
);

#endif
