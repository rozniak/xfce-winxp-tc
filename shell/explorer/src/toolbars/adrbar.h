#ifndef __ADRBAR_H__
#define __ADRBAR_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "../toolbar.h"
#include "../window.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExpAddressToolbarClass WinTCExpAddressToolbarClass;
typedef struct _WinTCExpAddressToolbar      WinTCExpAddressToolbar;

#define WINTC_TYPE_EXP_ADDRESS_TOOLBAR            (wintc_exp_address_toolbar_get_type())
#define WINTC_EXP_ADDRESS_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXP_ADDRESS_TOOLBAR, WinTCExpAddressToolbar))
#define WINTC_EXP_ADDRESS_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXP_ADDRESS_TOOLBAR, WinTCExpAddressToolbarClass))
#define IS_WINTC_EXP_ADDRESS_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXP_ADDRESS_TOOLBAR))
#define IS_WINTC_EXP_ADDRESS_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXP_ADDRESS_TOOLBAR))
#define WINTC_EXP_ADDRESS_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXP_ADDRESS_TOOLBAR, WinTCExpAddressToolbar))

GType wintc_exp_address_toolbar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerToolbar* wintc_exp_address_toolbar_new(
    WinTCExplorerWindow* wnd
);

#endif
