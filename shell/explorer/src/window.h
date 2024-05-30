#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/shellext.h>

#include "application.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExplorerWindowClass WinTCExplorerWindowClass;
typedef struct _WinTCExplorerWindow      WinTCExplorerWindow;

#define WINTC_TYPE_EXPLORER_WINDOW            (wintc_explorer_window_get_type())
#define WINTC_EXPLORER_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindow))
#define WINTC_EXPLORER_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindowClass))
#define IS_WINTC_EXPLORER_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXPLORER_WINDOW))
#define IS_WINTC_EXPLORER_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXPLORER_WINDOW))
#define WINTC_EXPLORER_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXPLORER_WINDOW, WinTCExplorerWindowClass))

GType wintc_explorer_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_explorer_window_new(
    WinTCExplorerApplication* app,
    WinTCShextHost*           shext_host
);

void wintc_explorer_window_get_location(
    WinTCExplorerWindow* wnd,
    WinTCShextPathInfo*  path_info
);

#endif
