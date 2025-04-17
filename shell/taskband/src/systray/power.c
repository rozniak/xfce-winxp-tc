#include <glib.h>
#include <gtk/gtk.h>
#include <libupower-glib/upower.h>
#include <wintc/comgtk.h>

#include "behaviour.h"
#include "power.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationPowerClass
{
    WinTCNotificationBehaviourClass __parent__;
};

struct _WinTCNotificationPower
{
    WinTCNotificationBehaviour __parent__;

    // Power stuff
    //
    UpClient* up_client;
    UpDevice* up_last_battery;
    gchar*    up_last_battery_path;
};

//
// FORWARD DECLARATIONS
//
static void wintc_notification_power_constructed(
    GObject* object
);
static void wintc_notification_power_finalize(
    GObject* object
);

static UpDeviceLevel battery_pct_to_enum(
    gdouble percentage
);
static void check_and_register_battery(
    WinTCNotificationPower* power,
    UpDevice*               device
);
static void update_battery_battery_level(
    WinTCNotificationPower* power,
    UpDevice*               device
);
static void update_client_on_battery(
    WinTCNotificationPower* power,
    UpClient*               client
);

static void on_up_client_device_added(
    UpClient* up_client,
    UpDevice* up_device,
    gpointer  user_data
);
static void on_up_client_device_removed(
    UpClient*    up_client,
    const gchar* object_path,
    gpointer     user_data
);
static void on_up_client_battery_notify(
    UpClient*   up_client,
    GParamSpec* pspec,
    gpointer    user_data
);
static void on_up_device_battery_notify(
    UpDevice*   up_device,
    GParamSpec* pspec,
    gpointer    user_data
);

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationPower,
    wintc_notification_power,
    WINTC_TYPE_NOTIFICATION_BEHAVIOUR
)

static void wintc_notification_power_class_init(
    WinTCNotificationPowerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_power_constructed;
    object_class->finalize    = wintc_notification_power_finalize;
}

static void wintc_notification_power_init(
    WINTC_UNUSED(WinTCNotificationPower* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_power_constructed(
    GObject* object
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(object);

    // Connect to upower, enumerate existing devices and attach signals for
    // picking up new ones
    //
    GPtrArray* all_devices;
    UpDevice*  device;

    power->up_client = up_client_new();
    WINTC_LOG_DEBUG("Connected to upower");

    all_devices = up_client_get_devices2(power->up_client);

    for (guint i = 0; i < all_devices->len; i++)
    {
        device = all_devices->pdata[i];

        check_and_register_battery(power, device);
    }

    g_signal_connect(
        power->up_client,
        "device-added",
        G_CALLBACK(on_up_client_device_added),
        power
    );
    g_signal_connect(
        power->up_client,
        "device-removed",
        G_CALLBACK(on_up_client_device_removed),
        power
    );
    g_signal_connect(
        power->up_client,
        "notify::on-battery",
        G_CALLBACK(on_up_client_battery_notify),
        power
    );

    update_client_on_battery(power, power->up_client);

    g_ptr_array_unref(all_devices);

    (G_OBJECT_CLASS(
        wintc_notification_power_parent_class
    ))->constructed(object);
}

static void wintc_notification_power_finalize(
    GObject* object
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(object);

    g_free(power->up_last_battery_path);

    (G_OBJECT_CLASS(
        wintc_notification_power_parent_class
    ))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCNotificationPower* wintc_notification_power_new(
    GtkWidget* widget_notif
)
{
    return WINTC_NOTIFICATION_POWER(
        g_object_new(
            WINTC_TYPE_NOTIFICATION_POWER,
            "widget-notif", widget_notif,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static UpDeviceLevel battery_pct_to_enum(
    gdouble percentage
)
{
    // FIXME: Not checked against Windows XP
    //
    if (percentage <= 15.0f)
    {
        return UP_DEVICE_LEVEL_CRITICAL;
    }
    else if (percentage <= 25.0f)
    {
        return UP_DEVICE_LEVEL_LOW;
    }
    else if (percentage <= 75.0f)
    {
        return UP_DEVICE_LEVEL_NORMAL;
    }
    else if (percentage <= 90.0f)
    {
        return UP_DEVICE_LEVEL_HIGH;
    }
    else
    {
        return UP_DEVICE_LEVEL_FULL;
    }
}

static void check_and_register_battery(
    WinTCNotificationPower* power,
    UpDevice*               device
)
{
    guint device_kind;

    g_object_get(
        device,
        "kind", &device_kind,
        NULL
    );

    if (device_kind != UP_DEVICE_KIND_BATTERY)
    {
        return;
    }

    update_battery_battery_level(power, device);

    g_signal_connect(
        device,
        "notify::percentage",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
    g_signal_connect(
        device,
        "notify::power-supply",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
    g_signal_connect(
        device,
        "notify::state",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
}

static void update_battery_battery_level(
    WinTCNotificationPower* power,
    UpDevice*               device
)
{
    gdouble  percentage;
    gboolean power_supply;
    guint    state;

    g_object_get(
        device,
        "percentage",   &percentage,
        "power-supply", &power_supply,
        "state",        &state,
        NULL
    );

    if (!power_supply)
    {
        return;
    }

    // Update icon based on levels
    //
    // We do not use 'battery-level' from the upower API, because it's not
    // reliable (reports UP_DEVICE_LEVEL_NONE) -- so just use percentage
    //
    const gchar* icon_name;

    UpDeviceLevel battery_level = battery_pct_to_enum(percentage);
    gboolean      is_charging   = state == UP_DEVICE_STATE_CHARGING;

    switch (battery_level)
    {
        case UP_DEVICE_LEVEL_CRITICAL:
            icon_name =
                is_charging ?
                    "battery-caution-charging" :
                    "battery-caution";
            break;

        case UP_DEVICE_LEVEL_LOW:
            icon_name =
                is_charging ?
                    "battery-low-charging" :
                    "battery-low";
            break;

        case UP_DEVICE_LEVEL_NORMAL:
            icon_name =
                is_charging ?
                    "battery-good-charging" :
                    "battery-good";
            break;

        case UP_DEVICE_LEVEL_HIGH:
        case UP_DEVICE_LEVEL_FULL:
            icon_name =
                is_charging ?
                    "battery-full-charging" :
                    "battery-full";
            break;

        default:
            icon_name = "battery-missing";
            break;
    }

    g_object_set(
        power,
        "icon-name", icon_name,
        NULL
    );

    // Keep track of last battery, for when switching to-from AC power
    //
    const gchar* this_battery_path = up_device_get_object_path(device);

    if (g_strcmp0(power->up_last_battery_path, this_battery_path) != 0)
    {
        g_free(power->up_last_battery_path);

        power->up_last_battery      = device;
        power->up_last_battery_path = g_strdup(this_battery_path);
    }
}

static void update_client_on_battery(
    WinTCNotificationPower* power,
    UpClient*               client
)
{
    if (up_client_get_on_battery(client))
    {
        if (power->up_last_battery)
        {
            update_battery_battery_level(power, power->up_last_battery);
        }
        else
        {
            g_object_set(
                power,
                "icon-name", "battery-missing",
                NULL
            );
        }
    }
    else
    {
        if (!power->up_last_battery)
        {
            g_object_set(
                power,
                "icon-name", "ac-adapter",
                NULL
            );
        }
    }
}

//
// CALLBACKS
//
static void on_up_client_device_added(
    WINTC_UNUSED(UpClient* up_client),
    UpDevice* up_device,
    gpointer  user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    check_and_register_battery(power, up_device);
}

static void on_up_client_device_removed(
    WINTC_UNUSED(UpClient*    up_client),
    const gchar* object_path,
    gpointer     user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    if (g_strcmp0(power->up_last_battery_path, object_path))
    {
        power->up_last_battery = NULL;
        g_free(power->up_last_battery_path);
    }
}

static void on_up_client_battery_notify(
    UpClient* up_client,
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer  user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    update_client_on_battery(power, up_client);
}

static void on_up_device_battery_notify(
    UpDevice* up_device,
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer  user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    update_battery_battery_level(power, up_device);
}
