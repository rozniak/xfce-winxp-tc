#include <errno.h>
#include <gio/gio.h>
#include <glib.h>
#include <glib-unix.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <wintc/comgtk.h>

#include "wpactrl.h"

//
// PRIVATE CONSTANTS
//
#define WPA_REPLY_MAX        8192
#define WPA_REQUEST_TIMEOUT_MS 5000
#define WPA_PING_INTERVAL_S  10

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_EVENT = 0,
    SIGNAL_CLOSED,
    N_SIGNALS
};

//
// FORWARD DECLARATIONS
//
static void wintc_wpa_monitor_dispose(
    GObject* object
);
static void wintc_wpa_monitor_finalize(
    GObject* object
);

static void wintc_wpa_monitor_close(
    WinTCWpaMonitor* monitor
);

static gint wpa_open_socket(
    const gchar* ctrl_path,
    gchar**      local_path,
    GError**     error
);

static gboolean on_monitor_readable(
    gint         fd,
    GIOCondition condition,
    gpointer     user_data
);
static gboolean on_timeout_ping(
    gpointer user_data
);

//
// STATIC DATA
//
static guint wintc_wpa_monitor_signals[N_SIGNALS] = { 0 };

static gint s_socket_counter = 0;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCWpaMonitor
{
    GObject __parent__;

    gint   fd;
    gchar* local_path;
    guint  id_watch;
    guint  id_ping;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCWpaMonitor,
    wintc_wpa_monitor,
    G_TYPE_OBJECT
)

static void wintc_wpa_monitor_class_init(
    WinTCWpaMonitorClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose  = wintc_wpa_monitor_dispose;
    object_class->finalize = wintc_wpa_monitor_finalize;

    wintc_wpa_monitor_signals[SIGNAL_EVENT] =
        g_signal_new(
            "event",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__STRING,
            G_TYPE_NONE,
            1,
            G_TYPE_STRING
        );
    wintc_wpa_monitor_signals[SIGNAL_CLOSED] =
        g_signal_new(
            "closed",
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

static void wintc_wpa_monitor_init(
    WinTCWpaMonitor* self
)
{
    self->fd = -1;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_wpa_monitor_dispose(
    GObject* object
)
{
    WinTCWpaMonitor* monitor = WINTC_WPA_MONITOR(object);

    if (monitor->fd >= 0)
    {
        static const gchar s_detach[] = "DETACH";

        send(monitor->fd, s_detach, sizeof(s_detach) - 1, MSG_DONTWAIT);
    }

    wintc_wpa_monitor_close(monitor);

    (G_OBJECT_CLASS(wintc_wpa_monitor_parent_class))->dispose(object);
}

static void wintc_wpa_monitor_finalize(
    GObject* object
)
{
    WinTCWpaMonitor* monitor = WINTC_WPA_MONITOR(object);

    g_free(monitor->local_path);

    (G_OBJECT_CLASS(wintc_wpa_monitor_parent_class))->finalize(object);
}

//
// PUBLIC FUNCTIONS
//
gchar* wintc_wpa_ctrl_request(
    const gchar* ctrl_path,
    const gchar* cmd,
    GError**     error
)
{
    gchar* local_path = NULL;
    gchar* reply      = NULL;
    gint   fd         = wpa_open_socket(ctrl_path, &local_path, error);

    if (fd < 0)
    {
        return NULL;
    }

    if (send(fd, cmd, strlen(cmd), 0) < 0)
    {
        gint err = errno;

        g_set_error(
            error,
            G_IO_ERROR,
            g_io_error_from_errno(err),
            "Failed to talk to wpa_supplicant: %s",
            g_strerror(err)
        );
    }
    else
    {
        gchar*        buf = g_malloc(WPA_REPLY_MAX + 1);
        struct pollfd pfd = { .fd = fd, .events = POLLIN, .revents = 0 };

        while (!reply)
        {
            gint ready = poll(&pfd, 1, WPA_REQUEST_TIMEOUT_MS);

            if (ready < 0 && errno == EINTR)
            {
                continue;
            }

            if (ready <= 0)
            {
                g_set_error_literal(
                    error,
                    G_IO_ERROR,
                    G_IO_ERROR_TIMED_OUT,
                    "wpa_supplicant did not reply."
                );
                break;
            }

            ssize_t len = recv(fd, buf, WPA_REPLY_MAX, 0);

            if (len < 0)
            {
                gint err = errno;

                g_set_error(
                    error,
                    G_IO_ERROR,
                    g_io_error_from_errno(err),
                    "Failed to read from wpa_supplicant: %s",
                    g_strerror(err)
                );
                break;
            }

            // Events only arrive on attached sockets, but skip any anyway
            //
            if (len > 0 && buf[0] == '<')
            {
                continue;
            }

            reply = g_strndup(buf, len);
        }

        g_free(buf);
    }

    close(fd);
    unlink(local_path);
    g_free(local_path);

    return reply;
}

gboolean wintc_wpa_ctrl_request_ok(
    const gchar* ctrl_path,
    const gchar* cmd,
    GError**     error
)
{
    gchar* reply = wintc_wpa_ctrl_request(ctrl_path, cmd, error);

    if (!reply)
    {
        return FALSE;
    }

    gboolean ok = g_str_has_prefix(reply, "OK");

    if (!ok)
    {
        // Only print the command name, it might carry a key
        //
        gchar** words = g_strsplit(cmd, " ", 2);

        g_set_error(
            error,
            G_IO_ERROR,
            G_IO_ERROR_FAILED,
            "wpa_supplicant refused %s: %s",
            words[0],
            g_strstrip(reply)
        );

        g_strfreev(words);
    }

    g_free(reply);

    return ok;
}

WinTCWpaMonitor* wintc_wpa_monitor_new(
    const gchar* ctrl_path,
    GError**     error
)
{
    gchar* local_path = NULL;
    gint   fd         = wpa_open_socket(ctrl_path, &local_path, error);

    if (fd < 0)
    {
        return NULL;
    }

    // Attach synchronously so that failing to talk to wpa_supplicant is
    // reported straight away
    //
    static const gchar s_attach[] = "ATTACH";

    gchar         reply[16];
    struct pollfd pfd = { .fd = fd, .events = POLLIN, .revents = 0 };
    ssize_t       len = -1;

    if (
        send(fd, s_attach, sizeof(s_attach) - 1, 0) >= 0 &&
        poll(&pfd, 1, WPA_REQUEST_TIMEOUT_MS) > 0
    )
    {
        len = recv(fd, reply, sizeof(reply) - 1, 0);
    }

    if (len < 2 || strncmp(reply, "OK", 2) != 0)
    {
        gint err = len < 0 ? errno : 0;

        g_set_error(
            error,
            G_IO_ERROR,
            err ? g_io_error_from_errno(err) : G_IO_ERROR_FAILED,
            "Failed to attach to wpa_supplicant: %s",
            err ? g_strerror(err) : "unexpected reply"
        );

        close(fd);
        unlink(local_path);
        g_free(local_path);

        return NULL;
    }

    g_unix_set_fd_nonblocking(fd, TRUE, NULL);

    WinTCWpaMonitor* monitor =
        WINTC_WPA_MONITOR(g_object_new(WINTC_TYPE_WPA_MONITOR, NULL));

    monitor->fd         = fd;
    monitor->local_path = local_path;
    monitor->id_watch   =
        g_unix_fd_add(
            fd,
            G_IO_IN | G_IO_ERR | G_IO_HUP,
            on_monitor_readable,
            monitor
        );

    // Nothing tells an attached socket that wpa_supplicant restarted, the
    // ping fails once it has though
    //
    monitor->id_ping =
        g_timeout_add_seconds(
            WPA_PING_INTERVAL_S,
            on_timeout_ping,
            monitor
        );

    return monitor;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_wpa_monitor_close(
    WinTCWpaMonitor* monitor
)
{
    g_clear_handle_id(&(monitor->id_watch), g_source_remove);
    g_clear_handle_id(&(monitor->id_ping),  g_source_remove);

    if (monitor->fd >= 0)
    {
        close(monitor->fd);
        unlink(monitor->local_path);

        monitor->fd = -1;
    }
}

static gint wpa_open_socket(
    const gchar* ctrl_path,
    gchar**      local_path,
    GError**     error
)
{
    struct sockaddr_un local = { 0 };
    struct sockaddr_un dest  = { 0 };

    gint fd = socket(PF_UNIX, SOCK_DGRAM, 0);

    if (fd < 0)
    {
        gint err = errno;

        g_set_error(
            error,
            G_IO_ERROR,
            g_io_error_from_errno(err),
            "Failed to create socket: %s",
            g_strerror(err)
        );

        return -1;
    }

    // wpa_supplicant replies to the address we send from, so the socket has
    // to be bound to a path - the same as wpa_ctrl.c does
    //
    *local_path =
        g_strdup_printf(
            "%s/wintc-wpa-%d-%d",
            g_get_tmp_dir(),
            (gint) getpid(),
            g_atomic_int_add(&s_socket_counter, 1)
        );

    local.sun_family = AF_UNIX;
    dest.sun_family  = AF_UNIX;

    if (
        g_strlcpy(
            local.sun_path,
            *local_path,
            sizeof(local.sun_path)
        ) >= sizeof(local.sun_path) ||
        g_strlcpy(
            dest.sun_path,
            ctrl_path,
            sizeof(dest.sun_path)
        ) >= sizeof(dest.sun_path)
    )
    {
        g_set_error_literal(
            error,
            G_IO_ERROR,
            G_IO_ERROR_FILENAME_TOO_LONG,
            "Socket path is too long."
        );

        close(fd);
        g_clear_pointer(local_path, g_free);

        return -1;
    }

    unlink(*local_path);

    if (
        bind(fd, (struct sockaddr*) &local, sizeof(local)) < 0 ||
        connect(fd, (struct sockaddr*) &dest, sizeof(dest)) < 0
    )
    {
        gint err = errno;

        g_set_error(
            error,
            G_IO_ERROR,
            g_io_error_from_errno(err),
            "Failed to connect to %s: %s",
            ctrl_path,
            g_strerror(err)
        );

        close(fd);
        unlink(*local_path);
        g_clear_pointer(local_path, g_free);

        return -1;
    }

    return fd;
}

//
// CALLBACKS
//
static gboolean on_monitor_readable(
    WINTC_UNUSED(gint fd),
    GIOCondition condition,
    gpointer     user_data
)
{
    WinTCWpaMonitor* monitor = WINTC_WPA_MONITOR(user_data);

    gchar    buf[WPA_REPLY_MAX + 1];
    gboolean closed = (condition & (G_IO_ERR | G_IO_HUP)) != 0;

    g_object_ref(monitor);

    while (!closed)
    {
        ssize_t len = recv(monitor->fd, buf, WPA_REPLY_MAX, 0);

        if (len < 0)
        {
            closed = errno != EAGAIN && errno != EINTR;
            break;
        }

        buf[len] = '\0';

        // Events are '<level>text', anything else is the reply to ATTACH or
        // PING
        //
        if (buf[0] != '<')
        {
            continue;
        }

        gchar* text = strchr(buf, '>');

        if (text)
        {
            g_signal_emit(
                monitor,
                wintc_wpa_monitor_signals[SIGNAL_EVENT],
                0,
                g_strchomp(text + 1)
            );
        }

        if (monitor->fd < 0)
        {
            break;
        }
    }

    // The monitor may have been closed by a handler of the event
    //
    gboolean keep = monitor->fd >= 0;

    if (closed && keep)
    {
        monitor->id_watch = 0;

        wintc_wpa_monitor_close(monitor);

        g_signal_emit(monitor, wintc_wpa_monitor_signals[SIGNAL_CLOSED], 0);

        keep = FALSE;
    }

    g_object_unref(monitor);

    return keep ? G_SOURCE_CONTINUE : G_SOURCE_REMOVE;
}

static gboolean on_timeout_ping(
    gpointer user_data
)
{
    WinTCWpaMonitor* monitor = WINTC_WPA_MONITOR(user_data);

    static const gchar s_ping[] = "PING";

    if (send(monitor->fd, s_ping, sizeof(s_ping) - 1, MSG_DONTWAIT) >= 0)
    {
        return G_SOURCE_CONTINUE;
    }

    monitor->id_ping = 0;

    wintc_wpa_monitor_close(monitor);

    g_signal_emit(monitor, wintc_wpa_monitor_signals[SIGNAL_CLOSED], 0);

    return G_SOURCE_REMOVE;
}
