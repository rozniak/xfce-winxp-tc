#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _TrayClockPrivate TrayClockPrivate;
typedef struct _TrayClockClass   TrayClockClass;
typedef struct _TrayClock        TrayClock;

#define TYPE_TRAY_CLOCK            (tray_clock_get_type())
#define TRAY_CLOCK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_TRAY_CLOCK, TrayClock))
#define TRAY_CLOCK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TRAY_CLOCK, TrayClockClass))
#define IS_TRAY_CLOCK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_TRAY_CLOCK))
#define IS_TRAY_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TRAY_CLOCK))
#define TRAY_CLOCK_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_TRAY_CLOCK, TrayClock))

GType tray_clock_get_type(void) G_GNUC_CONST;

G_END_DECLS

//
// PUBLIC FUNCTIONS
//
GtkWidget* tray_clock_new(void);

#endif
