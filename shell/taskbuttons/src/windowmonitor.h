#ifndef __WINDOWMONITOR_H__
#define __WINDOWMONITOR_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _WindowMonitor WindowMonitor;

//
// PUBLIC FUNCTIONS
//
WindowMonitor* window_monitor_init_management(
    GtkContainer* container
);

#endif
