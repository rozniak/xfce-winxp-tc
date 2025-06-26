#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_sm.h"
#include "../public/xfsm.h"

//
// FORWARD DECLARATIONS
//
static void wintc_gina_sm_xfce_igina_sm_interface_init(
    WinTCIGinaSmInterface* iface
);

static void wintc_gina_sm_xfce_dispose(
    GObject* object
);

gboolean wintc_gina_sm_xfce_is_valid(
    WinTCIGinaSm* sm
);

gboolean wintc_gina_sm_xfce_can_restart(
    WinTCIGinaSm* sm
);
gboolean wintc_gina_sm_xfce_can_shut_down(
    WinTCIGinaSm* sm
);
gboolean wintc_gina_sm_xfce_can_sleep(
    WinTCIGinaSm* sm
);

gboolean wintc_gina_sm_xfce_log_off(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xfce_restart(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xfce_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xfce_sleep(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xfce_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
);

static gboolean wintc_gina_sm_xfce_call_can(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name
);
static gboolean wintc_gina_sm_xfce_call_do(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name,
    GError**         error
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCGinaSmXfce
{
    GObject __parent__;

    // State stuff
    //
    GDBusProxy* proxy;
} WinTCGinaSmXfce;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCGinaSmXfce,
    wintc_gina_sm_xfce,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_IGINA_SM,
        wintc_gina_sm_xfce_igina_sm_interface_init
    )
)

static void wintc_gina_sm_xfce_class_init(
    WinTCGinaSmXfceClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_gina_sm_xfce_dispose;
}

static void wintc_gina_sm_xfce_init(
    WinTCGinaSmXfce* self
)
{
    GError* error = NULL;

    self->proxy =
        g_dbus_proxy_new_for_bus_sync(
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
            NULL,
            "org.xfce.SessionManager",
            "/org/xfce/SessionManager",
            "org.xfce.Session.Manager",
            NULL,
            &error
        );

    if (!(self->proxy))
    {
        wintc_log_error_and_clear(&error);
        return;
    }
}

static void wintc_gina_sm_xfce_igina_sm_interface_init(
    WinTCIGinaSmInterface* iface
)
{
    iface->is_valid      = wintc_gina_sm_xfce_is_valid;
    iface->can_restart   = wintc_gina_sm_xfce_can_restart;
    iface->can_shut_down = wintc_gina_sm_xfce_can_shut_down;
    iface->can_sleep     = wintc_gina_sm_xfce_can_sleep;
    iface->log_off       = wintc_gina_sm_xfce_log_off;
    iface->restart       = wintc_gina_sm_xfce_restart;
    iface->shut_down     = wintc_gina_sm_xfce_shut_down;
    iface->sleep         = wintc_gina_sm_xfce_sleep;
    iface->switch_user   = wintc_gina_sm_xfce_switch_user;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_gina_sm_xfce_dispose(
    GObject* object
)
{
    WinTCGinaSmXfce* sm_xfce = WINTC_GINA_SM_XFCE(object);

    g_clear_object(&(sm_xfce->proxy));

    (G_OBJECT_CLASS(wintc_gina_sm_xfce_parent_class))->dispose(object);
}

//
// INTERFACE METHODS (WinTCIGinaSm)
//
gboolean wintc_gina_sm_xfce_is_valid(
    WinTCIGinaSm* sm
)
{
    return !!((WINTC_GINA_SM_XFCE(sm))->proxy);
}

gboolean wintc_gina_sm_xfce_can_restart(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xfce_call_can(
        WINTC_GINA_SM_XFCE(sm),
        "CanRestart"
    );
}

gboolean wintc_gina_sm_xfce_can_shut_down(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xfce_call_can(
        WINTC_GINA_SM_XFCE(sm),
        "CanShutdown"
    );
}

gboolean wintc_gina_sm_xfce_can_sleep(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xfce_call_can(
        WINTC_GINA_SM_XFCE(sm),
        "CanHybridSleep"
    );
}

gboolean wintc_gina_sm_xfce_log_off(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    return wintc_gina_sm_xfce_call_do(
        WINTC_GINA_SM_XFCE(sm),
        "Logout",
        error
    );
}

gboolean wintc_gina_sm_xfce_restart(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            WINTC_GINA_SM_XFCE(sm),
            "CanRestart"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        WINTC_GINA_SM_XFCE(sm),
        "Restart",
        error
    );
}

gboolean wintc_gina_sm_xfce_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            WINTC_GINA_SM_XFCE(sm),
            "CanShutdown"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        WINTC_GINA_SM_XFCE(sm),
        "Shutdown",
        error
    );
}

gboolean wintc_gina_sm_xfce_sleep(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    if (
        !wintc_gina_sm_xfce_call_can(
            WINTC_GINA_SM_XFCE(sm),
            "CanHybridSleep"
        )
    )
    {
        return FALSE;
    }

    return wintc_gina_sm_xfce_call_do(
        WINTC_GINA_SM_XFCE(sm),
        "HyrbidSleep",
        error
    );
}

gboolean wintc_gina_sm_xfce_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    return wintc_gina_sm_xfce_call_do(
        WINTC_GINA_SM_XFCE(sm),
        "SwitchUser",
        error
    );
}

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXfce* wintc_gina_sm_xfce_new(void)
{
    return WINTC_GINA_SM_XFCE(
        g_object_new(
            WINTC_TYPE_GINA_SM_XFCE,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_gina_sm_xfce_call_can(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name
)
{
    if (!(sm_xfce->proxy))
    {
        WINTC_LOG_USER_DEBUG(
            "gina: attempted to call %s but no connection to xfsm",
            method_name
        );

        return FALSE;
    }

    GVariant* result_v =
        g_dbus_proxy_call_sync(
            sm_xfce->proxy,
            method_name,
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            NULL
        );

    gboolean result;

    g_variant_get(
        result_v,
        "(b)",
        &result
    );

    g_variant_unref(result_v);

    return result;
}

static gboolean wintc_gina_sm_xfce_call_do(
    WinTCGinaSmXfce* sm_xfce,
    const gchar*     method_name,
    GError**         error
)
{
    //
    // Assume the caller actually bothered to call the CanX method before this
    // one
    //
    GVariant* params_v = NULL;

    if (g_strcmp0(method_name, "Logout") == 0)
    {
        params_v = g_variant_new("(bb)", FALSE, FALSE);
    }
    else if (
        g_strcmp0(method_name, "Restart") == 0 ||
        g_strcmp0(method_name, "Shutdown") == 0
    )
    {
        params_v = g_variant_new("(b)", FALSE);
    }

    g_dbus_proxy_call_sync(
        sm_xfce->proxy,
        method_name,
        params_v,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    return !!error;
}
