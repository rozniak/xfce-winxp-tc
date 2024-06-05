#ifndef __WINDOWMONITOR_H__
#define __WINDOWMONITOR_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _WindowMonitor WindowMonitor;

//
// PUBLIC FUNCTIONS
//
void window_monitor_destroy(
    WindowMonitor* monitor
);

WindowMonitor* window_monitor_init_management(
    GtkContainer* container
);

G_END_DECLS

#endif
