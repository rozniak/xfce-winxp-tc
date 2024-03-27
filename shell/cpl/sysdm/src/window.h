#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplSysdmWindowClass WinTCCplSysdmWindowClass;
typedef struct _WinTCCplSysdmWindow      WinTCCplSysdmWindow;

#define WINTC_TYPE_CPL_SYSDM_WINDOW            (wintc_cpl_sysdm_window_get_type())
#define WINTC_CPL_SYSDM_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_SYSDM_WINDOW, WinTCCplSysdmWindow))
#define WINTC_CPL_SYSDM_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_SYSDM_WINDOW, WinTCCplSysdmWindowClass))
#define IS_WINTC_CPL_SYSDM_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_SYSDM_WINDOW))
#define IS_WINTC_CPL_SYSDM_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_SYSDM_WINDOW))
#define WINTC_CPL_SYSDM_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CPL_SYSDM_WINDOW, WinTCCplSysdmWindowClass))

GType wintc_cpl_sysdm_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_sysdm_window_new(
    WinTCCplSysdmApplication* app
);

#endif
