#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCClockRunnerClass WinTCClockRunnerClass;
typedef struct _WinTCClockRunner      WinTCClockRunner;

#define WINTC_TYPE_CLOCK_RUNNER            (wintc_clock_runner_get_type())
#define WINTC_CLOCK_RUNNER(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_CLOCK_RUNNER, WinTCClockRunner))
#define WINTC_CLOCK_RUNNER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_CLOCK_RUNNER, WinTCClockRunnerClass))
#define IS_WINTC_CLOCK_RUNNER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_CLOCK_RUNNER))
#define IS_WINTC_CLOCK_RUNNER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_CLOCK_RUNNER))
#define WINTC_CLOCK_RUNNER_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_CLOCK_RUNNER, WinTCClockRunner))

GType wintc_clock_runner_get_type(void) G_GNUC_CONST;

G_END_DECLS

//
// PUBLIC FUNCTIONS
//
WinTCClockRunner* wintc_clock_runner_new(
    GtkLabel* label_target
);

#endif
