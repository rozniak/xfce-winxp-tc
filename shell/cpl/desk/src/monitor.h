#ifndef __MONITOR_H__
#define __MONITOR_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCDeskMonitorClass WinTCDeskMonitorClass;
typedef struct _WinTCDeskMonitor      WinTCDeskMonitor;

#define WINTC_TYPE_DESK_MONITOR            (wintc_desk_monitor_get_type())
#define WINTC_DESK_MONITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_DESK_MONITOR, WinTCDeskMonitor))
#define WINTC_DESK_MONITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_DESK_MONITOR, WinTCDeskMonitorClass))
#define IS_WINTC_DESK_MONITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_DESK_MONITOR))
#define IS_WINTC_DESK_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_DESK_MONITOR))
#define WINTC_DESK_MONITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_DESK_MONITOR, WinTCDeskMonitorClass))

GType wintc_desk_monitor_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_desk_monitor_new(void);

void wintc_desk_monitor_set_pixbuf(
    WinTCDeskMonitor* monitor,
    const GdkPixbuf*  pixbuf
);

#endif
