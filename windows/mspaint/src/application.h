#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCPaintApplicationClass WinTCPaintApplicationClass;
typedef struct _WinTCPaintApplication      WinTCPaintApplication;

#define WINTC_TYPE_PAINT_APPLICATION            (wintc_paint_application_get_type())
#define WINTC_PAINT_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_PAINT_APPLICATION, WinTCPaintApplication))
#define WINTC_PAINT_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_PAINT_APPLICATION, WinTCPaintApplicationClass))
#define IS_WINTC_PAINT_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_PAINT_APPLICATION))
#define IS_WINTC_PAINT_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_PAINT_APPLICATION))
#define WINTC_PAINT_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_PAINT_APPLICATION, WinTCPaintApplicationClass))

GType wintc_paint_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCPaintApplication* wintc_paint_application_new(void);

#endif
