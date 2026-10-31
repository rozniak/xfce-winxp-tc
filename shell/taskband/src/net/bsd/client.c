#include <errno.h>
#include <glib.h>
#include <glib-unix.h>
#include <ifaddrs.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_media.h>
#include <net/route.h>
#include <unistd.h>
#include <wintc/comgtk.h>

#include "../client.h"
#include "../device.h"
#include "backend.h"

//
// PRIVATE CONSTANTS
//
#define NET_SYNC_DELAY_MS 100

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

static void wintc_net_client_queue_sync(
    WinTCNetClient* client
);
static void wintc_net_client_refresh_device(
    WinTCNetClient* client,
    guint           index
);
static void wintc_net_client_sync_devices(
    WinTCNetClient* client
);

static gboolean net_get_device_kind(
    gint                sock,
    const gchar*        iface,
    gchar**             cloners,
    WinTCNetDeviceKind* kind
);
static gchar** net_get_cloners(
    gint sock
);

static gboolean on_route_socket_readable(
    gint         fd,
    GIOCondition condition,
    gpointer     user_data
);
static gboolean on_timeout_sync(
    gpointer user_data
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

    GPtrArray* devices;

    gint  route_fd;
    guint id_route_watch;
    guint id_sync;
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
    self->devices = g_ptr_array_new_with_free_func(g_object_unref);

    // The routing socket tells us about interfaces coming and going, link
    // changes and address changes - the same thing devd(8) listens to
    //
    self->route_fd = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);

    if (self->route_fd < 0)
    {
        g_warning(
            "net: failed to open routing socket: %s",
            g_strerror(errno)
        );
    }
    else
    {
        g_unix_set_fd_nonblocking(self->route_fd, TRUE, NULL);

        self->id_route_watch =
            g_unix_fd_add(
                self->route_fd,
                G_IO_IN,
                on_route_socket_readable,
                self
            );
    }

    wintc_net_client_sync_devices(self);
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_net_client_dispose(
    GObject* object
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(object);

    g_clear_handle_id(&(client->id_route_watch), g_source_remove);
    g_clear_handle_id(&(client->id_sync),        g_source_remove);

    if (client->route_fd >= 0)
    {
        close(client->route_fd);
        client->route_fd = -1;
    }

    g_clear_pointer(&(client->devices), g_ptr_array_unref);

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
static void wintc_net_client_queue_sync(
    WinTCNetClient* client
)
{
    if (client->id_sync)
    {
        return;
    }

    client->id_sync =
        g_timeout_add(
            NET_SYNC_DELAY_MS,
            on_timeout_sync,
            client
        );
}

static void wintc_net_client_refresh_device(
    WinTCNetClient* client,
    guint           index
)
{
    gchar iface[IF_NAMESIZE];

    if (!index || !if_indextoname(index, iface))
    {
        return;
    }

    for (guint i = 0; i < client->devices->len; i++)
    {
        WinTCNetDevice* device = g_ptr_array_index(client->devices, i);

        if (g_strcmp0(wintc_net_device_get_interface(device), iface) == 0)
        {
            wintc_net_device_refresh(device);
            break;
        }
    }
}

static void wintc_net_client_sync_devices(
    WinTCNetClient* client
)
{
    struct ifaddrs* ifap    = NULL;
    GHashTable*     present =
        g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
    gboolean        changed = FALSE;
    gint            sock    = socket(AF_INET, SOCK_DGRAM, 0);
    gchar**         cloners = sock >= 0 ? net_get_cloners(sock) : NULL;

    if (sock >= 0 && getifaddrs(&ifap) == 0)
    {
        for (struct ifaddrs* ifa = ifap; ifa; ifa = ifa->ifa_next)
        {
            WinTCNetDeviceKind kind;

            if (
                !(ifa->ifa_addr)                    ||
                ifa->ifa_addr->sa_family != AF_LINK ||
                (ifa->ifa_flags & IFF_LOOPBACK)     ||
                !net_get_device_kind(sock, ifa->ifa_name, cloners, &kind)
            )
            {
                continue;
            }

            g_hash_table_insert(
                present,
                g_strdup(ifa->ifa_name),
                GINT_TO_POINTER(kind + 1)
            );
        }

        freeifaddrs(ifap);
    }

    if (sock >= 0)
    {
        close(sock);
    }

    g_strfreev(cloners);

    // Drop devices that went away...
    //
    for (guint i = client->devices->len; i > 0; i--)
    {
        WinTCNetDevice* device = g_ptr_array_index(client->devices, i - 1);
        const gchar*    iface  = wintc_net_device_get_interface(device);

        if (g_hash_table_contains(present, iface))
        {
            g_hash_table_remove(present, iface);
        }
        else
        {
            g_ptr_array_remove_index(client->devices, i - 1);
            changed = TRUE;
        }
    }

    // ...and pick up new ones
    //
    GHashTableIter iter;
    gpointer       key;
    gpointer       value;

    g_hash_table_iter_init(&iter, present);

    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        WINTC_LOG_DEBUG("net: found adapter %s", (gchar*) key);

        g_ptr_array_add(
            client->devices,
            wintc_net_device_new(
                (gchar*) key,
                (WinTCNetDeviceKind) (GPOINTER_TO_INT(value) - 1)
            )
        );
        changed = TRUE;
    }

    g_hash_table_unref(present);

    if (changed)
    {
        g_signal_emit(
            client,
            wintc_net_client_signals[SIGNAL_DEVICES_CHANGED],
            0
        );
    }
}

static gboolean net_get_device_kind(
    gint                sock,
    const gchar*        iface,
    gchar**             cloners,
    WinTCNetDeviceKind* kind
)
{
    struct ifmediareq ifmr = { 0 };

    g_strlcpy(ifmr.ifm_name, iface, sizeof(ifmr.ifm_name));

    // Bridges, tunnels, pflog etc. have no media, so they're out straight
    // away
    //
    if (ioctl(sock, SIOCGIFMEDIA, &ifmr) < 0)
    {
        return FALSE;
    }

    // wlan(4) interfaces are the ones to use for wireless, the hardware
    // underneath (iwlwifi0 etc.) isn't an interface
    //
    if (IFM_TYPE(ifmr.ifm_current) == IFM_IEEE80211)
    {
        *kind = WINTC_NET_DEVICE_KIND_WIRELESS;
        return TRUE;
    }

    if (IFM_TYPE(ifmr.ifm_current) != IFM_ETHER)
    {
        return FALSE;
    }

    // Ethernet interfaces made by a cloner are virtual (tap, epair, vlan,
    // lagg...), those aren't connections in their own right
    //
    gchar* driver = g_strdup(iface);
    gsize  len    = strlen(driver);

    while (len && g_ascii_isdigit(driver[len - 1]))
    {
        driver[--len] = '\0';
    }

    gboolean cloned =
        cloners && g_strv_contains((const gchar* const*) cloners, driver);

    g_free(driver);

    if (cloned)
    {
        return FALSE;
    }

    *kind = WINTC_NET_DEVICE_KIND_WIRED;
    return TRUE;
}

static gchar** net_get_cloners(
    gint sock
)
{
    struct if_clonereq ifcr = { 0 };

    if (ioctl(sock, SIOCIFGCLONERS, &ifcr) < 0 || ifcr.ifcr_total <= 0)
    {
        return NULL;
    }

    gchar* buf = g_malloc0((gsize) ifcr.ifcr_total * IFNAMSIZ);

    ifcr.ifcr_count  = ifcr.ifcr_total;
    ifcr.ifcr_buffer = buf;

    if (ioctl(sock, SIOCIFGCLONERS, &ifcr) < 0)
    {
        g_free(buf);
        return NULL;
    }

    gint    count   = MIN(ifcr.ifcr_count, ifcr.ifcr_total);
    gchar** cloners = g_new0(gchar*, count + 1);

    for (gint i = 0; i < count; i++)
    {
        cloners[i] = g_strndup(buf + (i * IFNAMSIZ), IFNAMSIZ);
    }

    g_free(buf);

    return cloners;
}

//
// CALLBACKS
//
static gboolean on_route_socket_readable(
    gint fd,
    WINTC_UNUSED(GIOCondition condition),
    gpointer user_data
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(user_data);

    // Each read gives one message
    //
    union
    {
        struct rt_msghdr rtm;
        gchar            buf[2048];
    } msg;

    for (;;)
    {
        ssize_t len = recv(fd, &msg, sizeof(msg), 0);

        if (len < 0)
        {
            // The kernel drops messages when we fall behind - check
            // everything again
            //
            if (errno == ENOBUFS)
            {
                wintc_net_client_queue_sync(client);

                for (guint i = 0; i < client->devices->len; i++)
                {
                    wintc_net_device_refresh(
                        g_ptr_array_index(client->devices, i)
                    );
                }

                continue;
            }

            break;
        }

        // Every message starts with the length, version and type - the rest
        // depends on the type, and most are shorter than a struct rt_msghdr
        //
        if (
            len < (ssize_t) G_STRUCT_OFFSET(struct rt_msghdr, rtm_index) ||
            msg.rtm.rtm_version != RTM_VERSION
        )
        {
            continue;
        }

        switch (msg.rtm.rtm_type)
        {
            case RTM_IFANNOUNCE:
                wintc_net_client_queue_sync(client);
                break;

            case RTM_IFINFO:
                if (len >= (ssize_t) sizeof(struct if_msghdr))
                {
                    wintc_net_client_refresh_device(
                        client,
                        ((struct if_msghdr*) &msg)->ifm_index
                    );
                }
                break;

            case RTM_NEWADDR:
            case RTM_DELADDR:
                if (len >= (ssize_t) sizeof(struct ifa_msghdr))
                {
                    wintc_net_client_refresh_device(
                        client,
                        ((struct ifa_msghdr*) &msg)->ifam_index
                    );
                }
                break;

            case RTM_ADD:
            case RTM_DELETE:
            case RTM_CHANGE:
                // The default route is the gateway shown in the status
                // window
                //
                if (len >= (ssize_t) sizeof(struct rt_msghdr))
                {
                    wintc_net_client_refresh_device(
                        client,
                        msg.rtm.rtm_index
                    );
                }
                break;

            default:
                break;
        }
    }

    return G_SOURCE_CONTINUE;
}

static gboolean on_timeout_sync(
    gpointer user_data
)
{
    WinTCNetClient* client = WINTC_NET_CLIENT(user_data);

    client->id_sync = 0;

    wintc_net_client_sync_devices(client);

    return G_SOURCE_REMOVE;
}
