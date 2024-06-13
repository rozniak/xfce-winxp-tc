#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "application.h"
#include "settings.h"

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCCplDeskWindowClass
{
    GtkApplicationWindowClass __parent__;
} WinTCCplDeskWindowClass;

typedef struct _WinTCCplDeskWindow
{
    GtkApplicationWindow __parent__;

    // State
    //
    WinTCCplDeskSettings* settings;
    gboolean              sync_settings;

    // UI
    //
    GtkWidget* notebook_main;

    // Desktop page
    //
    GSList* list_wallpapers;

    GtkWidget* combo_style;
    GtkWidget* listbox_wallpapers;
    GtkWidget* monitor_desktop;

    GdkPixbuf* pixbuf_wallpaper;
} WinTCCplDeskWindow;

#define WINTC_TYPE_CPL_DESK_WINDOW            (wintc_cpl_desk_window_get_type())
#define WINTC_CPL_DESK_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CPL_DESK_WINDOW, WinTCCplDeskWindow))
#define WINTC_CPL_DESK_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CPL_DESK_WINDOW, WinTCCplDeskWindowClass))
#define IS_WINTC_CPL_DESK_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CPL_DESK_WINDOW))
#define IS_WINTC_CPL_DESK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CPL_DESK_WINDOW))
#define WINTC_CPL_DESK_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CPL_DESK_WINDOW, WinTCCplDeskWindowClass))

GType wintc_cpl_desk_window_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_cpl_desk_window_new(
    WinTCCplDeskApplication* app
);

#endif
