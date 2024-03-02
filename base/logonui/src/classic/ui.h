#ifndef __CLASSIC_UI_H__
#define __CLASSIC_UI_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCClassicUIClass WinTCClassicUIClass;
typedef struct _WinTCClassicUI      WinTCClassicUI;

#define WINTC_TYPE_CLASSIC_UI            (wintc_classic_ui_get_type())
#define WINTC_CLASSIC_UI(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CLASSIC_UI, WinTCClassicUI))
#define WINTC_CLASSIC_UI_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((obj), WINTC_TYPE_CLASSIC_UI, WinTCClassicUI))
#define IS_WINTC_CLASSIC_UI(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CLASSIC_UI))
#define IS_WINTC_CLASSIC_UI_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CLASSIC_UI))
#define WINTC_CLASSIC_UI_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CLASSIC_UI))

GType wintc_classic_ui_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_classic_ui_new(void);

#endif
