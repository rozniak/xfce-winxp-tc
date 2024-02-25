#ifndef __TOOLBAR_START_H__
#define __TOOLBAR_START_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
#define TYPE_WINTC_TOOLBAR_START            (wintc_toolbar_start_get_type())
#define WINTC_TOOLBAR_START(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_TOOLBAR_START, WinTCToolbarStart))
#define WINTC_TOOLBAR_START_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_TOOLBAR_START, WinTCToolbarStartClass))
#define IS_WINTC_TOOLBAR_START(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_TOOLBAR_START))
#define IS_WINTC_TOOLBAR_START_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_TOOLBAR_START))
#define WINTC_TOOLBAR_START_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_TOOLBAR_START, WinTCToolbarStart))

GType wintc_toolbar_start_get_type(void) G_GNUC_CONST;

#endif

