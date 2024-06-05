#include <glib.h>
#include <gtk/gtk.h>

#include "behaviour.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_WIDGET_NOTIF = 1,
    PROP_ICON_NAME
};

enum
{
    SIGNAL_ICON_CHANGED = 0,
    N_SIGNALS
};

//
// STATIC DATA
//
static gint wintc_notification_behaviour_signals[N_SIGNALS] = { 0 };

//
// FORWARD DECLARATIONS
//
static void wintc_notification_behaviour_finalize(
    GObject* object
);
static void wintc_notification_behaviour_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_notification_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationBehaviour,
    wintc_notification_behaviour,
    G_TYPE_OBJECT
)

static void wintc_notification_behaviour_class_init(
    WinTCNotificationBehaviourClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->finalize     = wintc_notification_behaviour_finalize;
    object_class->get_property = wintc_notification_behaviour_get_property;
    object_class->set_property = wintc_notification_behaviour_set_property;

    wintc_notification_behaviour_signals[SIGNAL_ICON_CHANGED] =
        g_signal_new(
            "icon-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );

    g_object_class_install_property(
        object_class,
        PROP_WIDGET_NOTIF,
        g_param_spec_object(
            "widget-notif",
            "WidgetNotif",
            "The GTK widget that hosts the notification icon.",
            GTK_TYPE_WIDGET,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_ICON_NAME,
        g_param_spec_string(
            "icon-name",
            "IconName",
            "The icon name to display in the notification area.",
            NULL,
            G_PARAM_WRITABLE
        )
    );
}

static void wintc_notification_behaviour_init(
    WinTCNotificationBehaviour* self
)
{
    self->icon_name    = NULL;
    self->widget_notif = NULL;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_behaviour_finalize(
    GObject* object
)
{
    WinTCNotificationBehaviour* behaviour =
        WINTC_NOTIFICATION_BEHAVIOUR(object);

    g_free(behaviour->icon_name);

    (G_OBJECT_CLASS(wintc_notification_behaviour_parent_class))
        ->finalize(object);
}

static void wintc_notification_behaviour_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCNotificationBehaviour* behaviour =
        WINTC_NOTIFICATION_BEHAVIOUR(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            g_value_set_string(value, behaviour->icon_name);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_notification_behaviour_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCNotificationBehaviour* behaviour =
        WINTC_NOTIFICATION_BEHAVIOUR(object);

    switch (prop_id)
    {
        case PROP_ICON_NAME:
            g_free(behaviour->icon_name);
            behaviour->icon_name = g_strdup(g_value_get_string(value));

            g_signal_emit(
                behaviour,
                wintc_notification_behaviour_signals[SIGNAL_ICON_CHANGED],
                0
            );

            break;

        case PROP_WIDGET_NOTIF:
            behaviour->widget_notif = GTK_WIDGET(g_value_get_object(value));

            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_notification_behaviour_get_icon_name(
    WinTCNotificationBehaviour* behaviour
)
{
    return behaviour->icon_name;
}
