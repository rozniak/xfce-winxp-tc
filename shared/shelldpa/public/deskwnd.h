#ifndef __SHELLDPA_DESKWND_H__
#define __SHELLDPA_DESKWND_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDpaDesktopWindowClass
{
    GtkApplicationWindowClass __parent__;
} WinTCDpaDesktopWindowClass;

typedef struct _WinTCDpaDesktopWindow
{
    GtkApplicationWindow __parent__;

    GdkMonitor* monitor;
} WinTCDpaDesktopWindow;

#define WINTC_TYPE_DPA_DESKTOP_WINDOW (wintc_dpa_desktop_window_get_type())
#define WINTC_DPA_DESKTOP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_DPA_DESKTOP_WINDOW, WinTCDpaDesktopWindow))
#define WINTC_DPA_DESKTOP_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_DPA_DESKTOP_WINDOW, WinTCDpaDesktopWindow))
#define IS_WINTC_DPA_DESKTOP_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_DPA_DESKTOP_WINDOW))
#define IS_WINTC_DPA_DESKTOP_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_DPA_DESKTOP_WINDOW))
#define WINTC_DPA_DESKTOP_WINDOW_GET_CLASS(obj) (G_TYPE_CHECK_INSTANCE_GET_CLASS((obj), WINTC_TYPE_DPA_DESKTOP_WINDOW))

GType wintc_dpa_desktop_window_get_type(void) G_GNUC_CONST;

#endif
