#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCTaskbandWindowClass   WinTCTaskbandWindowClass;
typedef struct _WinTCTaskbandWindow        WinTCTaskbandWindow;

#define WINTC_TYPE_TASKBAND_WINDOW            (wintc_taskband_window_get_type())
#define WINTC_TASKBAND_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_TASKBAND_WINDOW, WinTCTaskbandWindow))
#define WINTC_TASKBAND_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_TASKBAND_WINDOW, WinTCTaskbandWindow))
#define IS_WINTC_TASKBAND_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_TASKBAND_WINDOW))
#define IS_WINTC_TASKBAND_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_TASKBAND_WINDOW))
#define WINTC_TASKBAND_WINDOW_GET_CLASS(obj)  (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_TASKBAND_WINDOW))

GType wintc_taskband_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskband_window_new(
    WinTCTaskbandApplication* app
);

#endif

