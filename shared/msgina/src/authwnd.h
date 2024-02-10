#ifndef __AUTHWND_H__
#define __AUTHWND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCGinaAuthWindowPrivate WinTCGinaAuthWindowPrivate;
typedef struct _WinTCGinaAuthWindowClass   WinTCGinaAuthWindowClass;
typedef struct _WinTCGinaAuthWindow        WinTCGinaAuthWindow;

#define TYPE_WINTC_GINA_AUTH_WINDOW            (wintc_gina_auth_window_get_type())
#define WINTC_GINA_AUTH_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define WINTC_GINA_AUTH_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define IS_WINTC_GINA_AUTH_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_GINA_AUTH_WINDOW))
#define IS_WINTC_GINA_AUTH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_GINA_AUTH_WINDOW))
#define WINTC_GINA_WINDOW_GET_CLASS(obj)       (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), TYPE_WINTC_GINA_AUTH_WINDOW))

GType wintc_gina_auth_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_gina_auth_window_new(void);

#endif

