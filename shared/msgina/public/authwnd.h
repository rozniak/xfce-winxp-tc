/** @file */

#ifndef __MSGINA_AUTHWND_H__
#define __MSGINA_AUTHWND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCGinaAuthWindowClass WinTCGinaAuthWindowClass;

/**
 * A standard GINA authentication window.
 */
typedef struct _WinTCGinaAuthWindow WinTCGinaAuthWindow;

#define WINTC_TYPE_GINA_AUTH_WINDOW            (wintc_gina_auth_window_get_type())
#define WINTC_GINA_AUTH_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define WINTC_GINA_AUTH_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_GINA_AUTH_WINDOW, WinTCGinaAuthWindow))
#define IS_WINTC_GINA_AUTH_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_GINA_AUTH_WINDOW))
#define IS_WINTC_GINA_AUTH_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_GINA_AUTH_WINDOW))
#define WINTC_GINA_WINDOW_GET_CLASS(obj)       (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_GINA_AUTH_WINDOW))

GType wintc_gina_auth_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//

/**
 * Creates a new instance of WinTCGinaAuthWindow.
 *
 * @return The new WinTCGinaAuthWindow instance cast to GtkWidget.
 */
GtkWidget* wintc_gina_auth_window_new(void);

#endif

