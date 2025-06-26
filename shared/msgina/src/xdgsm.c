#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_sm.h"
#include "../public/xdgsm.h"

//
// FORWARD DECLARATIONS
//
static void wintc_gina_sm_xdg_igina_sm_interface_init(
    WinTCIGinaSmInterface* iface
);

static void wintc_gina_sm_xdg_dispose(
    GObject* object
);

gboolean wintc_gina_sm_xdg_is_valid(
    WinTCIGinaSm* sm
);

gboolean wintc_gina_sm_xdg_can_restart(
    WinTCIGinaSm* sm
);
gboolean wintc_gina_sm_xdg_can_shut_down(
    WinTCIGinaSm* sm
);
gboolean wintc_gina_sm_xdg_can_sleep(
    WinTCIGinaSm* sm
);

gboolean wintc_gina_sm_xdg_log_off(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xdg_restart(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xdg_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xdg_sleep(
    WinTCIGinaSm* sm,
    GError**      error
);
gboolean wintc_gina_sm_xdg_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
);

gboolean wintc_gina_sm_xdg_call_can(
    WinTCGinaSmXdg* sm_xdg,
    const gchar*       method_name
);
gboolean wintc_gina_sm_xdg_call_do(
    WinTCGinaSmXdg* sm_xdg,
    const gchar*    method_name,
    GError**        error
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCGinaSmXdg
{
    GObject __parent__;

    // State stuff
    //
    GDBusProxy* proxy_dm;
    GDBusProxy* proxy_logind;
} WinTCGinaSmXdg;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCGinaSmXdg,
    wintc_gina_sm_xdg,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_IGINA_SM,
        wintc_gina_sm_xdg_igina_sm_interface_init
    )
)

static void wintc_gina_sm_xdg_class_init(
    WinTCGinaSmXdgClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_gina_sm_xdg_dispose;
}

static void wintc_gina_sm_xdg_init(
    WinTCGinaSmXdg* self
)
{
    GError* error_dm     = NULL;
    GError* error_logind = NULL;

    if (g_getenv("XDG_SEAT_PATH"))
    {
        self->proxy_dm     =
            g_dbus_proxy_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM,
                G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                NULL,
                "org.freedesktop.DisplayManager",
                g_getenv("XDG_SEAT_PATH"),
                "org.freedesktop.DisplayManager.Seat",
                NULL,
                &error_dm
            );
    }

    self->proxy_logind =
        g_dbus_proxy_new_for_bus_sync(
            G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
            NULL,
            "org.freedesktop.login1",
            "/org/freedesktop/login1",
            "org.freedesktop.login1.Manager",
            NULL,
            &error_logind
        );

    wintc_log_error_and_clear(&error_dm);
    wintc_log_error_and_clear(&error_logind);
}

static void wintc_gina_sm_xdg_igina_sm_interface_init(
    WinTCIGinaSmInterface* iface
)
{
    iface->is_valid      = wintc_gina_sm_xdg_is_valid;
    iface->can_restart   = wintc_gina_sm_xdg_can_restart;
    iface->can_shut_down = wintc_gina_sm_xdg_can_shut_down;
    iface->can_sleep     = wintc_gina_sm_xdg_can_sleep;
    iface->log_off       = wintc_gina_sm_xdg_log_off;
    iface->restart       = wintc_gina_sm_xdg_restart;
    iface->shut_down     = wintc_gina_sm_xdg_shut_down;
    iface->sleep         = wintc_gina_sm_xdg_sleep;
    iface->switch_user   = wintc_gina_sm_xdg_switch_user;
}

//
// CLASS VIRTUAL MEHODS
//
static void wintc_gina_sm_xdg_dispose(
    GObject* object
)
{
    WinTCGinaSmXdg* sm_xdg = WINTC_GINA_SM_XDG(object);

    g_clear_object(&(sm_xdg->proxy_dm));
    g_clear_object(&(sm_xdg->proxy_logind));

    (G_OBJECT_CLASS(wintc_gina_sm_xdg_parent_class))->dispose(object);
}

//
// INTERFACE METHODS (WinTCIGinaSm)
//
gboolean wintc_gina_sm_xdg_is_valid(
    WinTCIGinaSm* sm
)
{
    WinTCGinaSmXdg* sm_xdg = WINTC_GINA_SM_XDG(sm);

    return !!(sm_xdg->proxy_logind);
}

gboolean wintc_gina_sm_xdg_can_restart(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xdg_call_can(
        WINTC_GINA_SM_XDG(sm),
        "CanReboot"
    );
}

gboolean wintc_gina_sm_xdg_can_shut_down(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xdg_call_can(
        WINTC_GINA_SM_XDG(sm),
        "CanPowerOff"
    );
}

gboolean wintc_gina_sm_xdg_can_sleep(
    WinTCIGinaSm* sm
)
{
    return wintc_gina_sm_xdg_call_can(
        WINTC_GINA_SM_XDG(sm),
        "CanHybridSleep"
    );
}

gboolean wintc_gina_sm_xdg_log_off(
    WINTC_UNUSED(WinTCIGinaSm* sm),
    WINTC_UNUSED(GError**      error)
)
{
    //
    // FIXME: Should probably emit a signal to be picked up by the session
    //        manager implementation
    //
    g_critical("%s", "gina: SmXdg: session manager would log out now");
    return FALSE;
}

gboolean wintc_gina_sm_xdg_restart(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    return wintc_gina_sm_xdg_call_do(
        WINTC_GINA_SM_XDG(sm),
        "Reboot",
        error
    );
}

gboolean wintc_gina_sm_xdg_shut_down(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    return wintc_gina_sm_xdg_call_do(
        WINTC_GINA_SM_XDG(sm),
        "PowerOff",
        error
    );
}

gboolean wintc_gina_sm_xdg_sleep(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    return wintc_gina_sm_xdg_call_do(
        WINTC_GINA_SM_XDG(sm),
        "HybridSleep",
        error
    );
}

gboolean wintc_gina_sm_xdg_switch_user(
    WinTCIGinaSm* sm,
    GError**      error
)
{
    WinTCGinaSmXdg* sm_xdg = WINTC_GINA_SM_XDG(sm);

    if (!(sm_xdg->proxy_dm))
    {
        // FIXME: Return error
        return FALSE;
    }

    g_dbus_proxy_call_sync(
        sm_xdg->proxy_dm,
        "SwitchToGreeter",
        NULL,
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    return !!error;
}

//
// PUBLIC FUNCTIONS
//
WinTCGinaSmXdg* wintc_gina_sm_xdg_new(void)
{
    return WINTC_GINA_SM_XDG(
        g_object_new(
            WINTC_TYPE_GINA_SM_XDG,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
gboolean wintc_gina_sm_xdg_call_can(
    WinTCGinaSmXdg* sm_xdg,
    const gchar*    method_name
)
{
    GError* error = NULL;

    if (!(sm_xdg->proxy_logind))
    {
        WINTC_LOG_USER_DEBUG(
            "gina: attempted to call %s but no connection to logind",
            method_name
        );

        return FALSE;
    }

    gboolean  result;
    gchar*    result_s = NULL;
    GVariant* result_v =
        g_dbus_proxy_call_sync(
            sm_xdg->proxy_logind,
            method_name,
            NULL,
            G_DBUS_CALL_FLAGS_NONE,
            -1,
            NULL,
            NULL
        );

    if (!result_v)
    {
        wintc_log_error_and_clear(&error);
        return FALSE;
    }

    g_variant_get(
        result_v,
        "(s)",
        &result_s
    );

    result = g_strcmp0(result_s, "yes") == 0;

    g_variant_unref(result_v);
    g_free(result_s);

    return result;
}

gboolean wintc_gina_sm_xdg_call_do(
    WinTCGinaSmXdg* sm_xdg,
    const gchar*    method_name,
    GError**        error
)
{
    g_dbus_proxy_call_sync(
        sm_xdg->proxy_logind,
        method_name,
        g_variant_new("(b)", FALSE), // For the 'interactive' parameter
        G_DBUS_CALL_FLAGS_NONE,
        -1,
        NULL,
        error
    );

    return !!error;
}
