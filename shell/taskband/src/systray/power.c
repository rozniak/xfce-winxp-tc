#include <glib.h>
#include <gtk/gtk.h>
#include <libupower-glib/upower.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "icon.h"
#include "power.h"

//
// FORWARD DECLARATIONS
//
static void wintc_notification_power_constructed(
    GObject* object
);
static void wintc_notification_power_dispose(
    GObject* object
);

static void wintc_notification_power_register_main_battery(
    WinTCNotificationPower* power
);
static void wintc_notification_power_update_icon(
    WinTCNotificationPower* power
);

static UpDeviceLevel battery_pct_to_enum(
    gdouble percentage
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
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationPower
{
    WinTCShextUIController __parent__;

    // UI
    //
    GtkWidget* notif_icon;

    // Power stuff
    //
    UpClient* up_client;
    UpDevice* up_device_battery;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNotificationPower,
    wintc_notification_power,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_notification_power_class_init(
    WinTCNotificationPowerClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_notification_power_constructed;
    object_class->dispose     = wintc_notification_power_dispose;
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
    (G_OBJECT_CLASS(wintc_notification_power_parent_class))
        ->constructed(object);

    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(object);

    power->notif_icon =
        wintc_ishext_ui_host_get_ext_widget(
            wintc_shext_ui_controller_get_ui_host(
                WINTC_SHEXT_UI_CONTROLLER(object)
            ),
            WINTC_NOTIFAREA_HOSTEXT_ICON,
            WINTC_TYPE_NOTIF_AREA_ICON,
            object
        );

    // Connect to upower, enumerate existing devices and attach signals for
    // picking up new ones
    //
    power->up_client = up_client_new();

    if (!power->up_client)
    {
        g_warning("%s", "taskband: power: Failed to connect to upower");
        return;
    }

    WINTC_LOG_DEBUG("taskband: power: Connected to upower");

    wintc_notification_power_register_main_battery(power);

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
}

static void wintc_notification_power_dispose(
    GObject* object
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(object);

    g_clear_object(&(power->up_device_battery));
    g_clear_object(&(power->up_client));

    (G_OBJECT_CLASS(wintc_notification_power_parent_class))
        ->dispose(object);
}

//
// PRIVATE FUNCTIONS
//
static void wintc_notification_power_register_main_battery(
    WinTCNotificationPower* power
)
{
    // Reset state (for our sanity really)
    //
    g_clear_object(&(power->up_device_battery));

    // ATM we can only raise one notification icon so just try to find 'a'
    // battery and make that the main one
    //
    GPtrArray* all_devices = up_client_get_devices2(power->up_client);
    gboolean   on_battery  = up_client_get_on_battery(power->up_client);

    for (guint i = 0; i < all_devices->len; i++)
    {
        UpDevice* device = (UpDevice*) all_devices->pdata[i];
        guint     device_kind;
        gboolean  power_supply;
        guint     state;

        g_object_get(
            device,
            "kind",         &device_kind,
            "power-supply", &power_supply,
            "state",        &state,
            NULL
        );

        if (device_kind == UP_DEVICE_KIND_BATTERY)
        {
            // This is the battery we want if:
            //   - We're on battery, and this is the battery providing power
            //   - We're not on battery, and this battery is charging
            //
            if (
                (on_battery && power_supply) ||
                (
                    !on_battery &&
                    (
                        state == UP_DEVICE_STATE_CHARGING ||
                        state == UP_DEVICE_STATE_FULLY_CHARGED
                    )
                )
            )
            {
                power->up_device_battery = g_object_ref(device);
                break;
            }
        }
    }

    g_ptr_array_unref(all_devices);

    // Update the icon for current battery state
    //
    wintc_notification_power_update_icon(power);

    // Connect up signals to monitor battery, assuming we have one?
    //
    if (!(power->up_device_battery))
    {
        return;
    }

    g_signal_connect(
        power->up_device_battery,
        "notify::percentage",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
    g_signal_connect(
        power->up_device_battery,
        "notify::power-supply",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
    g_signal_connect(
        power->up_device_battery,
        "notify::state",
        G_CALLBACK(on_up_device_battery_notify),
        power
    );
}

static void wintc_notification_power_update_icon(
    WinTCNotificationPower* power
)
{
    // Handle the situation where we have no battery, even if we're supposedly
    // on battery supply
    //
    if (!(power->up_device_battery))
    {
        if (up_client_get_on_battery(power->up_client))
        {
            wintc_notif_area_icon_set_icon_name(
                WINTC_NOTIF_AREA_ICON(power->notif_icon),
                "battery-missing"
            );
        }
        else
        {
            wintc_notif_area_icon_set_icon_name(
                WINTC_NOTIF_AREA_ICON(power->notif_icon),
                "ac-adapter"
            );
        }

        return;
    }

    // Okay we have a battery, so update the icon for it
    //
    // We do not use 'battery-level' from the upower API, because it's not
    // reliable (reports UP_DEVICE_LEVEL_NONE) -- so just use percentage
    //
    gdouble  percentage;
    guint    state;

    g_object_get(
        power->up_device_battery,
        "percentage",   &percentage,
        "state",        &state,
        NULL
    );

    UpDeviceLevel battery_level = battery_pct_to_enum(percentage);
    const gchar*  icon_name;
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

    wintc_notif_area_icon_set_icon_name(
        WINTC_NOTIF_AREA_ICON(power->notif_icon),
        icon_name
    );
}

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

//
// CALLBACKS
//
static void on_up_client_device_added(
    WINTC_UNUSED(UpClient* up_client),
    WINTC_UNUSED(UpDevice* up_device),
    gpointer user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    wintc_notification_power_register_main_battery(power);
}

static void on_up_client_device_removed(
    WINTC_UNUSED(UpClient* up_client),
    WINTC_UNUSED(const gchar* object_path),
    gpointer user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    wintc_notification_power_register_main_battery(power);
}

static void on_up_client_battery_notify(
    WINTC_UNUSED(UpClient* up_client),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    wintc_notification_power_register_main_battery(power);
}

static void on_up_device_battery_notify(
    WINTC_UNUSED(UpDevice* up_device),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    WinTCNotificationPower* power = WINTC_NOTIFICATION_POWER(user_data);

    wintc_notification_power_update_icon(power);
}
