#include <glib.h>
#include <gtk/gtk.h>

#include "clock.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _TrayClockPrivate
{
    TrayClock* tray_clock;

    guint      clock_source_id;
};

struct _TrayClockClass
{
    GtkLabelClass __parent__;
};

struct _TrayClock
{
    GtkLabel __parent__;

    TrayClockPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
static void tray_clock_finalize(
    GObject* object
);

static void tray_clock_launch_time(
    TrayClock* tray_clock
);
static void tray_clock_update_time(
    TrayClock* tray_clock
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
G_DEFINE_TYPE_WITH_CODE(
    TrayClock,
    tray_clock,
    GTK_TYPE_LABEL,
    G_ADD_PRIVATE(TrayClock)
)

static void tray_clock_class_init(
    TrayClockClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = tray_clock_finalize;
}

static void tray_clock_init(
    TrayClock* self
)
{
    self->priv = tray_clock_get_instance_private(self);

    // Add style class
    //
    GtkStyleContext* style = gtk_widget_get_style_context(GTK_WIDGET(self));

    gtk_style_context_add_class(style, "clock");

    // Establish clock - if we're not dead on the minute then delay launch to sync
    // up
    //
    GDateTime* time  = g_date_time_new_now_local();
    gint       delay = 60 - g_date_time_get_second(time);

    if (delay > 0)
    {
        tray_clock_update_time(self);

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
        tray_clock_launch_time(self);
    }

    g_date_time_unref(time);
}

//
// FINALIZE
//
static void tray_clock_finalize(
    GObject* object
)
{
    TrayClock* tray_clock = TRAY_CLOCK(object);

    if (tray_clock->priv->clock_source_id > 0)
    {
        g_source_remove(tray_clock->priv->clock_source_id);
    }

    (*G_OBJECT_CLASS(tray_clock_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* tray_clock_new(void)
{
    return GTK_WIDGET(
        g_object_new(TYPE_TRAY_CLOCK, NULL)
    );
}

//
// PRIVATE FUNCTIONS
//
static void tray_clock_launch_time(
    TrayClock* tray_clock
)
{
    tray_clock_update_time(tray_clock);

    tray_clock->priv->clock_source_id =
        g_timeout_add_seconds_full(
            G_PRIORITY_DEFAULT,
            60,
            on_clock_timeout_elapsed,
            tray_clock,
            NULL
        );
}

static void tray_clock_update_time(
    TrayClock* tray_clock
)
{
    GDateTime* time    = g_date_time_new_now_local();
    gchar*     timestr = g_date_time_format(time, "%H:%M");

    gtk_label_set_text(GTK_LABEL(tray_clock), timestr);

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
    tray_clock_update_time(TRAY_CLOCK(data));

    return TRUE;
}

static gboolean on_sync_timeout_elapsed(
    gpointer data
)
{
    tray_clock_launch_time(TRAY_CLOCK(data));

    return FALSE;
}
