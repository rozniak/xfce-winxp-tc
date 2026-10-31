#include <glib.h>
#include <NetworkManager.h>
#include <wintc/comgtk.h>

#include "../client.h"
#include "../device.h"
#include "backend.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_DEVICES_CHANGED = 0,
    N_SIGNALS
};

//
// FORWARD DECLARATIONS
//
static void wintc_net_client_dispose(
    GObject* object
);

static void wintc_net_client_sync_devices(
    WinTCNetClient* client
);

static gboolean net_device_is_shown(
    NMDevice* nm_device
);

static void on_nm_client_device_added(
    NMClient* nm_client,
    NMDevice* nm_device,
    gpointer  user_data
);
static void on_nm_client_device_removed(
    NMClient* nm_client,
    NMDevice* nm_device,
    gpointer  user_data
);
static void on_nm_client_ready(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void on_nm_device_notify_state(
    NMDevice*   nm_device,
    GParamSpec* pspec,
    gpointer    user_data
);

//
// STATIC DATA
//
static guint wintc_net_client_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNetClient
{
    GObject __parent__;

    NMClient*     nm_client;
    GCancellable* cancellable;
    gboolean      disposed;

    GPtrArray*  devices;
    GHashTable* map_watched_devices;

    gulong sigid_device_added;
    gulong sigid_device_removed;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCNetClient,
    wintc_net_client,
    G_TYPE_OBJECT
)

static void wintc_net_client_class_init(
    WinTCNetClientClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_net_client_dispose;

    wintc_net_client_signals[SIGNAL_DEVICES_CHANGED] =
        g_signal_new(
            "devices-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
}

static void wintc_net_client_init(
    WinTCNetClient* self
)
{
    self->cancellable         = g_cancellable_new();
    self->devices             = g_ptr_array_new_with_free_func(g_object_unref);
    self->map_watched_devices =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            g_object_unref,
            NULL
        );

    // Connect to NetworkManager
    //
    WINTC_LOG_DEBUG("net: connecting to NM...");

    nm_client_new_async(
        self->cancellable,
        (GAsyncReadyCallback) on_nm_client_ready,
        g_object_ref(self)
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_net_client_dispose(
    GObject* object
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(object);

    client->disposed = TRUE;

    if (client->cancellable)
    {
        g_cancellable_cancel(client->cancellable);
    }

    if (client->map_watched_devices)
    {
        GHashTableIter iter;
        gpointer       key;
        gpointer       value;

        g_hash_table_iter_init(&iter, client->map_watched_devices);

        while (g_hash_table_iter_next(&iter, &key, &value))
        {
            g_signal_handler_disconnect(key, GPOINTER_TO_SIZE(value));
        }

        g_clear_pointer(
            &(client->map_watched_devices),
            g_hash_table_unref
        );
    }

    if (client->nm_client)
    {
        g_clear_signal_handler(
            &(client->sigid_device_added),
            client->nm_client
        );
        g_clear_signal_handler(
            &(client->sigid_device_removed),
            client->nm_client
        );
    }

    g_clear_pointer(&(client->devices), g_ptr_array_unref);
    g_clear_object(&(client->cancellable));
    g_clear_object(&(client->nm_client));

    (G_OBJECT_CLASS(wintc_net_client_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCNetClient* wintc_net_client_new(void)
{
    return WINTC_NET_CLIENT(
        g_object_new(
            WINTC_TYPE_NET_CLIENT,
            NULL
        )
    );
}

GPtrArray* wintc_net_client_get_devices(
    WinTCNetClient* client
)
{
    return client->devices;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_net_client_sync_devices(
    WinTCNetClient* client
)
{
    const GPtrArray* nm_devices = nm_client_get_devices(client->nm_client);
    GPtrArray*       shown      = g_ptr_array_new();
    gboolean         changed    = FALSE;

    for (guint i = 0; nm_devices && i < nm_devices->len; i++)
    {
        NMDevice* nm_device = g_ptr_array_index(nm_devices, i);

        if (!NM_IS_DEVICE_ETHERNET(nm_device) && !NM_IS_DEVICE_WIFI(nm_device))
        {
            continue;
        }

        // Watch for devices becoming (un)managed
        //
        if (!g_hash_table_contains(client->map_watched_devices, nm_device))
        {
            gulong sigid =
                g_signal_connect(
                    nm_device,
                    "notify::" NM_DEVICE_STATE,
                    G_CALLBACK(on_nm_device_notify_state),
                    client
                );

            g_hash_table_insert(
                client->map_watched_devices,
                g_object_ref(nm_device),
                GSIZE_TO_POINTER(sigid)
            );
        }

        if (net_device_is_shown(nm_device))
        {
            g_ptr_array_add(shown, nm_device);
        }
    }

    // Drop devices that went away...
    //
    for (guint i = client->devices->len; i > 0; i--)
    {
        WinTCNetDevice* device = g_ptr_array_index(client->devices, i - 1);

        if (
            !g_ptr_array_find(
                shown,
                wintc_net_device_get_nm_device(device),
                NULL
            )
        )
        {
            g_ptr_array_remove_index(client->devices, i - 1);
            changed = TRUE;
        }
    }

    // ...and pick up new ones
    //
    for (guint i = 0; i < shown->len; i++)
    {
        NMDevice* nm_device = g_ptr_array_index(shown, i);
        gboolean  known     = FALSE;

        for (guint j = 0; j < client->devices->len; j++)
        {
            WinTCNetDevice* device = g_ptr_array_index(client->devices, j);

            if (wintc_net_device_get_nm_device(device) == nm_device)
            {
                known = TRUE;
                break;
            }
        }

        if (!known)
        {
            g_ptr_array_add(
                client->devices,
                wintc_net_device_new(client->nm_client, nm_device)
            );
            changed = TRUE;
        }
    }

    g_ptr_array_unref(shown);

    if (changed)
    {
        g_signal_emit(
            client,
            wintc_net_client_signals[SIGNAL_DEVICES_CHANGED],
            0
        );
    }
}

static gboolean net_device_is_shown(
    NMDevice* nm_device
)
{
    // Only show adapters NetworkManager manages - anything else isn't ours to
    // control (eg. container interfaces)
    //
    return nm_device_is_real(nm_device) &&
           nm_device_get_state(nm_device) != NM_DEVICE_STATE_UNMANAGED &&
           nm_device_get_state(nm_device) != NM_DEVICE_STATE_UNKNOWN;
}

//
// CALLBACKS
//
static void on_nm_client_device_added(
    WINTC_UNUSED(NMClient* nm_client),
    WINTC_UNUSED(NMDevice* nm_device),
    gpointer user_data
)
{
    wintc_net_client_sync_devices(WINTC_NET_CLIENT(user_data));
}

static void on_nm_client_device_removed(
    WINTC_UNUSED(NMClient* nm_client),
    NMDevice* nm_device,
    gpointer  user_data
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(user_data);

    gpointer sigid;

    if (
        g_hash_table_lookup_extended(
            client->map_watched_devices,
            nm_device,
            NULL,
            &sigid
        )
    )
    {
        g_signal_handler_disconnect(nm_device, GPOINTER_TO_SIZE(sigid));
        g_hash_table_remove(client->map_watched_devices, nm_device);
    }

    wintc_net_client_sync_devices(client);
}

static void on_nm_client_ready(
    WINTC_UNUSED(GObject* source_object),
    GAsyncResult* res,
    gpointer      user_data
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(user_data);

    // Finish up async connection
    //
    GError*   error     = NULL;
    NMClient* nm_client = nm_client_new_finish(res, &error);

    if (client->disposed)
    {
        g_clear_error(&error);
        g_clear_object(&nm_client);
        g_object_unref(client);
        return;
    }

    if (!nm_client)
    {
        wintc_log_error_and_clear(&error);
        g_object_unref(client);
        return;
    }

    WINTC_LOG_DEBUG("net: connected to NM");

    client->nm_client = nm_client;

    // NMClient keeps track of NetworkManager restarting, the devices come
    // and go with it
    //
    client->sigid_device_added =
        g_signal_connect(
            nm_client,
            "device-added",
            G_CALLBACK(on_nm_client_device_added),
            client
        );
    client->sigid_device_removed =
        g_signal_connect(
            nm_client,
            "device-removed",
            G_CALLBACK(on_nm_client_device_removed),
            client
        );

    wintc_net_client_sync_devices(client);

    g_object_unref(client);
}

static void on_nm_device_notify_state(
    WINTC_UNUSED(NMDevice*   nm_device),
    WINTC_UNUSED(GParamSpec* pspec),
    gpointer user_data
)
{
    wintc_net_client_sync_devices(WINTC_NET_CLIENT(user_data));
}
