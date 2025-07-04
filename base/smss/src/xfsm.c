#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/msgina.h>

#include "xfsm.h"
#include "xfsm-dbus.h"

//
// PRIVATE ENUMS
//
enum
{
    SIGNAL_VALID_CHANGED = 0,
    SIGNAL_LOGOUT_REQUESTED,
    N_SIGNALS
};

//
// FORWARD DECLARATIONS
//
static void wintc_smss_xfsm_host_dispose(
    GObject* object
);

static gboolean on_handle_can_hibernate(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_can_hybrid_sleep(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_can_restart(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_can_shutdown(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_can_suspend(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_hibernate(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_hybrid_sleep(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_logout(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gboolean               show_dialog,
    gboolean               allow_save,
    gpointer               user_data
);
static gboolean on_handle_restart(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gboolean               allow_save,
    gpointer               user_data
);
static gboolean on_handle_shutdown(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gboolean               allow_save,
    gpointer               user_data
);
static gboolean on_handle_suspend(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);
static gboolean on_handle_switch_user(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
);

static void on_name_acquired(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);
static void on_name_lost(
    GDBusConnection* connection,
    const gchar*     name,
    gpointer         user_data
);

//
// STATIC DATA
//
static gint wintc_smss_xfsm_host_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCSmssXfsmHost
{
    GObject __parent__;

    // State stuff
    //
    ZWinXfsmManager* dbus_xfsm;
    WinTCGinaSmXdg*  sm_xdg;
} WinTCSmssXfsmHost;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCSmssXfsmHost,
    wintc_smss_xfsm_host,
    G_TYPE_OBJECT
)

static void wintc_smss_xfsm_host_class_init(
    WinTCSmssXfsmHostClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_smss_xfsm_host_dispose;

    wintc_smss_xfsm_host_signals[SIGNAL_VALID_CHANGED] =
        g_signal_new(
            "valid-changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
        );
    wintc_smss_xfsm_host_signals[SIGNAL_LOGOUT_REQUESTED] =
        g_signal_new(
            "logout-requested",
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

static void wintc_smss_xfsm_host_init(
    WinTCSmssXfsmHost* self
)
{
    WINTC_LOG_DEBUG("smss: hosting name %s", "org.xfce.SessionManager");

    g_bus_own_name(
        G_BUS_TYPE_SESSION,
        "org.xfce.SessionManager",
        G_BUS_NAME_OWNER_FLAGS_NONE,
        NULL,
        on_name_acquired,
        on_name_lost,
        self,
        NULL
    );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_smss_xfsm_host_dispose(
    GObject* object
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(object);

    g_clear_object(&(host->dbus_xfsm));
    g_clear_object(&(host->sm_xdg));

    (G_OBJECT_CLASS(wintc_smss_xfsm_host_parent_class))
        ->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCSmssXfsmHost* wintc_smss_xfsm_host_new(void)
{
    return WINTC_SMSS_XFSM_HOST(
        g_object_new(
            WINTC_TYPE_SMSS_XFSM_HOST,
            NULL
        )
    );
}

gboolean wintc_smss_xfsm_host_is_valid(
    WinTCSmssXfsmHost* host
)
{
    if (!(host->dbus_xfsm))
    {
        g_critical("%s", "smss: xfsm: no dbus host");
        return FALSE;
    }
    if (!(host->sm_xdg))
    {
        g_critical("%s", "smss: xfsm: no xdg sm");
        return FALSE;
    }
    return host->dbus_xfsm && host->sm_xdg;
}

//
// CALLBACKS
//
static gboolean on_handle_can_hibernate(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("smss: xfsm received CanHibernate");

    zwin_xfsm_manager_complete_can_hibernate(
        dbus_xfsm,
        invocation,
        FALSE
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_can_hybrid_sleep(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received CanHybridSleep");

    zwin_xfsm_manager_complete_can_hybrid_sleep(
        dbus_xfsm,
        invocation,
        wintc_igina_sm_can_sleep(
            WINTC_IGINA_SM(host->sm_xdg)
        )
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_can_restart(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received CanRestart");

    zwin_xfsm_manager_complete_can_restart(
        dbus_xfsm,
        invocation,
        wintc_igina_sm_can_restart(
            WINTC_IGINA_SM(host->sm_xdg)
        )
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_can_shutdown(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received CanShutdown");

    zwin_xfsm_manager_complete_can_shutdown(
        dbus_xfsm,
        invocation,
        wintc_igina_sm_can_shut_down(
            WINTC_IGINA_SM(host->sm_xdg)
        )
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_can_suspend(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("smss: xfsm received CanSuspend");

    zwin_xfsm_manager_complete_can_shutdown(
        dbus_xfsm,
        invocation,
        FALSE
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_hibernate(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("smss: xfsm received Hibernate");

    zwin_xfsm_manager_complete_hibernate(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_hybrid_sleep(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received HybridSleep");

    GError* error = NULL;

    if (
        !wintc_igina_sm_sleep(
            WINTC_IGINA_SM(host->sm_xdg),
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    zwin_xfsm_manager_complete_hybrid_sleep(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_logout(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gboolean show_dialog),
    WINTC_UNUSED(gboolean allow_save),
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received Logout");

    g_signal_emit(
        host,
        wintc_smss_xfsm_host_signals[SIGNAL_LOGOUT_REQUESTED],
        0
    );

    zwin_xfsm_manager_complete_logout(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_restart(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gboolean allow_save),
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received Restart");

    GError* error = NULL;

    if (
        !wintc_igina_sm_restart(
            WINTC_IGINA_SM(host->sm_xdg),
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    zwin_xfsm_manager_complete_restart(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_shutdown(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gboolean allow_save),
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received Shutdown");

    GError* error = NULL;

    if (
        !wintc_igina_sm_shut_down(
            WINTC_IGINA_SM(host->sm_xdg),
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    zwin_xfsm_manager_complete_shutdown(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_suspend(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG("smss: xfsm received Suspend");

    zwin_xfsm_manager_complete_suspend(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static gboolean on_handle_switch_user(
    ZWinXfsmManager*       dbus_xfsm,
    GDBusMethodInvocation* invocation,
    gpointer               user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    WINTC_LOG_DEBUG("smss: xfsm received SwitchUser");

    GError* error = NULL;

    if (
        !wintc_igina_sm_switch_user(
            WINTC_IGINA_SM(host->sm_xdg),
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
    }

    zwin_xfsm_manager_complete_switch_user(
        dbus_xfsm,
        invocation
    );

    return G_DBUS_METHOD_INVOCATION_HANDLED;
}

static void on_name_acquired(
    GDBusConnection* connection,
    WINTC_UNUSED(const gchar* name),
    gpointer         user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    GError* error = NULL;

    // Create the DBus API host
    //
    ZWinXfsmManager* dbus_xfsm = zwin_xfsm_manager_skeleton_new();

    g_signal_connect(
        dbus_xfsm,
        "handle-can-hibernate",
        G_CALLBACK(on_handle_can_hibernate),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-can-hybrid-sleep",
        G_CALLBACK(on_handle_can_hybrid_sleep),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-can-restart",
        G_CALLBACK(on_handle_can_restart),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-can-shutdown",
        G_CALLBACK(on_handle_can_shutdown),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-can-suspend",
        G_CALLBACK(on_handle_can_suspend),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-hibernate",
        G_CALLBACK(on_handle_hibernate),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-hybrid-sleep",
        G_CALLBACK(on_handle_hybrid_sleep),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-logout",
        G_CALLBACK(on_handle_logout),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-restart",
        G_CALLBACK(on_handle_restart),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-shutdown",
        G_CALLBACK(on_handle_shutdown),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-suspend",
        G_CALLBACK(on_handle_suspend),
        host
    );
    g_signal_connect(
        dbus_xfsm,
        "handle-switch-user",
        G_CALLBACK(on_handle_switch_user),
        host
    );

    if (
        !g_dbus_interface_skeleton_export(
            G_DBUS_INTERFACE_SKELETON(dbus_xfsm),
            connection,
            "/org/xfce/SessionManager",
            &error
        )
    )
    {
        wintc_log_error_and_clear(&error);
        g_clear_object(&dbus_xfsm);
    }

    host->dbus_xfsm = dbus_xfsm;

    // Create the XDG SM connection
    //
    WinTCGinaSmXdg* sm_xdg = wintc_gina_sm_xdg_new();

    if (
        !wintc_igina_sm_is_valid(
            WINTC_IGINA_SM(sm_xdg)
        )
    )
    {
        g_clear_object(&sm_xdg);
    }

    host->sm_xdg = sm_xdg;

    // Ping the validity check for the owner
    //
    g_signal_emit(
        host,
        wintc_smss_xfsm_host_signals[SIGNAL_VALID_CHANGED],
        0
    );
}

static void on_name_lost(
    WINTC_UNUSED(GDBusConnection* connection),
    WINTC_UNUSED(const gchar*     name),
    gpointer user_data
)
{
    WinTCSmssXfsmHost* host = WINTC_SMSS_XFSM_HOST(user_data);

    g_signal_emit(
        host,
        wintc_smss_xfsm_host_signals[SIGNAL_VALID_CHANGED],
        0
    );
}
