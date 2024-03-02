#include <glib.h>
#include <gtk/gtk.h>

#include "clock.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_LABEL_TARGET = 1,
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCClockRunnerClass
{
    GObjectClass __parent__;
};

struct _WinTCClockRunner
{
    GObject __parent__;

    guint     clock_source_id;
    GtkLabel* label_target;
};

//
// FORWARD DECLARATIONS
//
static void wintc_clock_runner_finalize(
    GObject* object
);
static void wintc_clock_runner_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_clock_runner_launch_time(
    WinTCClockRunner* tray_clock
);
static void wintc_clock_runner_update_time(
    WinTCClockRunner* tray_clock
);

static gboolean on_clock_timeout_elapsed(
    gpointer data
);
static gboolean on_sync_timeout_elapsed(
    gpointer data
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCClockRunner,
    wintc_clock_runner,
    G_TYPE_OBJECT
)

static void wintc_clock_runner_class_init(
    WinTCClockRunnerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_clock_runner_finalize;
    object_class->set_property = wintc_clock_runner_set_property;

    g_object_class_install_property(
        object_class,
        PROP_LABEL_TARGET,
        g_param_spec_object(
            "label-target",
            "LabelTarget",
            "The target label to manage.",
            GTK_TYPE_LABEL,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}

static void wintc_clock_runner_init(
    WinTCClockRunner* self
)
{
    // Establish clock - if we're not dead on the minute then delay launch to sync
    // up
    //
    GDateTime* time  = g_date_time_new_now_local();
    gint       delay = 60 - g_date_time_get_second(time);

    if (delay > 0)
    {
        wintc_clock_runner_update_time(self);

        g_timeout_add_seconds_full(
            G_PRIORITY_DEFAULT,
            delay,
            on_sync_timeout_elapsed,
            self,
            NULL
        );
    }
    else
    {
        wintc_clock_runner_launch_time(self);
    }

    g_date_time_unref(time);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_clock_runner_finalize(
    GObject* object
)
{
    WinTCClockRunner* clock_runner = WINTC_CLOCK_RUNNER(object);

    if (clock_runner->clock_source_id > 0)
    {
        g_source_remove(clock_runner->clock_source_id);
    }

    (*G_OBJECT_CLASS(wintc_clock_runner_parent_class)->finalize) (object);
}

static void wintc_clock_runner_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCClockRunner* clock_runner = WINTC_CLOCK_RUNNER(object);

    switch (prop_id)
    {
        case PROP_LABEL_TARGET:
            clock_runner->label_target =
                GTK_LABEL(g_value_get_object(value));

            wintc_clock_runner_update_time(clock_runner);

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCClockRunner* wintc_clock_runner_new(
    GtkLabel* label_target
)
{
    return WINTC_CLOCK_RUNNER(
        g_object_new(
            WINTC_TYPE_CLOCK_RUNNER,
            "label-target", label_target,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_clock_runner_launch_time(
    WinTCClockRunner* clock_runner
)
{
    wintc_clock_runner_update_time(clock_runner);

    clock_runner->clock_source_id =
        g_timeout_add_seconds_full(
            G_PRIORITY_DEFAULT,
            60,
            on_clock_timeout_elapsed,
            clock_runner,
            NULL
        );
}

static void wintc_clock_runner_update_time(
    WinTCClockRunner* clock_runner
)
{
    if (clock_runner->label_target == NULL)
    {
        return;
    }

    // Update target label with time
    //
    GDateTime* time    = g_date_time_new_now_local();
    gchar*     timestr = g_date_time_format(time, "%H:%M");

    gtk_label_set_text(
        GTK_LABEL(clock_runner->label_target),
        timestr
    );

    g_date_time_unref(time);
    g_free(timestr);
}

//
// CALLBACKS
//
static gboolean on_clock_timeout_elapsed(
    gpointer data
)
{
    wintc_clock_runner_update_time(WINTC_CLOCK_RUNNER(data));

    return TRUE;
}

static gboolean on_sync_timeout_elapsed(
    gpointer data
)
{
    wintc_clock_runner_launch_time(WINTC_CLOCK_RUNNER(data));

    return FALSE;
}
