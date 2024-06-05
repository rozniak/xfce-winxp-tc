#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "behaviour.h"
#include "clock.h"
#include "notifarea.h"
#include "volume.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationAreaClass
{
    GtkBinClass __parent__;
};

struct _WinTCNotificationArea
{
    GtkBin __parent__;

    GtkWidget* box_container;
    GtkWidget* label_clock;

    WinTCClockRunner* clock_runner;

    GHashTable* map_widget_to_behaviour;
};

//
// FORWARD DECLARATIONS
//
static void wintc_notification_area_dispose(
    GObject* object
);

static GtkWidget* wintc_notification_area_append_icon(
    WinTCNotificationArea* notif_area
);
static void wintc_notification_area_map_widget(
    WinTCNotificationArea*      notif_area,
    GtkWidget*                  widget_notif,
    WinTCNotificationBehaviour* behaviour
);

static void update_notification_icon(
    GtkWidget*                  widget_notif,
    WinTCNotificationBehaviour* behaviour
);

static void on_behaviour_icon_changed(
    WinTCNotificationBehaviour* behaviour,
    gpointer                    user_data
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationArea,
    wintc_notification_area,
    GTK_TYPE_BIN
)

static void wintc_notification_area_class_init(
    WinTCNotificationAreaClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_notification_area_dispose;
}

static void wintc_notification_area_init(
    WinTCNotificationArea* self
)
{
    // Create map for notification widgets --> behaviours
    //
    self->map_widget_to_behaviour =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            g_object_unref
        );

    // Set up UI
    //
    self->box_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->label_clock   = gtk_label_new(NULL);

    wintc_widget_add_style_class(self->box_container, "wintc-systray");
    wintc_widget_add_style_class(self->label_clock,   "clock");

    gtk_box_pack_end(
        GTK_BOX(self->box_container),
        self->label_clock,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        self->box_container
    );

    // Hook up clock runner
    //
    self->clock_runner =
        wintc_clock_runner_new(GTK_LABEL(self->label_clock));

    // Create volume icon and behaviour for system tray
    //
    WinTCNotificationVolume* notif_volume;
    GtkWidget*               widget_volume;

    widget_volume = wintc_notification_area_append_icon(self);
    notif_volume  = wintc_notification_volume_new(widget_volume);

    wintc_notification_area_map_widget(
        self,
        widget_volume,
        WINTC_NOTIFICATION_BEHAVIOUR(notif_volume)
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_area_dispose(
    GObject* object
)
{
    WinTCNotificationArea* notif_area = WINTC_NOTIFICATION_AREA(object);

    g_clear_object(&(notif_area->clock_runner));

    if (notif_area->map_widget_to_behaviour)
    {
        g_hash_table_unref(
            g_steal_pointer(&(notif_area->map_widget_to_behaviour))
        );
    }

    (G_OBJECT_CLASS(wintc_notification_area_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* notification_area_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTIFICATION_AREA,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static GtkWidget* wintc_notification_area_append_icon(
    WinTCNotificationArea* notif_area
)
{
    GtkWidget* event_box  = gtk_event_box_new();
    GtkWidget* image_icon = gtk_image_new();

    gtk_widget_set_events(event_box, GDK_BUTTON_PRESS_MASK);

    gtk_container_add(
        GTK_CONTAINER(event_box),
        image_icon
    );
    gtk_box_pack_start(
        GTK_BOX(notif_area->box_container),
        event_box,
        FALSE,
        FALSE,
        0
    );

    return event_box;
}

static void wintc_notification_area_map_widget(
    WinTCNotificationArea*      notif_area,
    GtkWidget*                  widget_notif,
    WinTCNotificationBehaviour* behaviour
)
{
    g_hash_table_insert(
        notif_area->map_widget_to_behaviour,
        widget_notif,
        behaviour
    );

    // Connect up widget to behaviour
    //
    update_notification_icon(
        widget_notif,
        behaviour
    );

    g_signal_connect(
        behaviour,
        "icon-changed",
        G_CALLBACK(on_behaviour_icon_changed),
        widget_notif
    );
}

static void update_notification_icon(
    GtkWidget*                  widget_notif,
    WinTCNotificationBehaviour* behaviour
)
{
    gtk_image_set_from_icon_name(
        GTK_IMAGE(gtk_bin_get_child(GTK_BIN(widget_notif))),
        wintc_notification_behaviour_get_icon_name(behaviour),
        GTK_ICON_SIZE_SMALL_TOOLBAR
    );
}

//
// CALLBACKS
//
static void on_behaviour_icon_changed(
    WinTCNotificationBehaviour* behaviour,
    gpointer                    user_data
)
{
    GtkWidget* widget_notif = GTK_WIDGET(user_data);

    update_notification_icon(
        widget_notif,
        behaviour
    );
}
