#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/reg-dbus.h"
#include "../public/regwrap.h"

//
// PRIVATE STRUCTS
//
typedef struct _RegistryCBData
{
    WinTCRegistryKeyCallback callback;
    gpointer                 user_data;
} RegistryCBData;

//
// FORWARD DECLARATIONS
//
static void wintc_registry_dispose(
    GObject* object
);

static gboolean wintc_registry_ensure_proxy(
    WinTCRegistry* registry,
    GError**       error
);

static void on_reg_key_value_changed(
    ZWinRegistryGDBUS* proxy,
    const gchar*       key_path,
    const gchar*       value_name,
    GVariant*          value_data,
    gpointer           user_data
);

//
// GLIB OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCRegistryClass
{
    GObjectClass __parent__;
};

struct _WinTCRegistry
{
    GObject __parent__;

    gboolean           connected_signal;
    GHashTable*        map_key_to_cb;
    ZWinRegistryGDBUS* proxy;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCRegistry,
    wintc_registry,
    G_TYPE_OBJECT
)

static void wintc_registry_class_init(
    WinTCRegistryClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_registry_dispose;
}

static void wintc_registry_init(
    WinTCRegistry* self
)
{
    self->map_key_to_cb =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            g_free
        );
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_registry_dispose(
    GObject* object
)
{
    WinTCRegistry* registry = WINTC_REGISTRY(object);

    g_hash_table_destroy(
        registry->map_key_to_cb
    );

    (G_OBJECT_CLASS(wintc_registry_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
WinTCRegistry* wintc_registry_new(void)
{
    return WINTC_REGISTRY(
        g_object_new(
            WINTC_TYPE_REGISTRY,
            NULL
        )
    );
}

gboolean wintc_registry_create_key(
    WinTCRegistry* registry,
    const gchar*   key_path,
    GError**       error
)
{
    GError* local_error = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    if (!wintc_registry_ensure_proxy(registry, &local_error))
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    // Perform dbus call
    //
    gint result = 0;

    if (
        !zwin_registry_gdbus_call_create_key_sync(
            registry->proxy,
            key_path,
            &result,
            NULL,
            &local_error
        )
    )
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    return !!result;
}

gboolean wintc_registry_get_key_value(
    WinTCRegistry*         registry,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data,
    GError**               error
)
{
    GError* local_error = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    switch (value_type)
    {
        case WINTC_REG_DWORD: *((gint*) value_data)   = 0;    break;
        case WINTC_REG_QWORD: *((gint64*) value_data) = 0;    break;
        case WINTC_REG_SZ:    *((gchar**) value_data) = NULL; break;

        default:
            g_critical("registry: unknown type %d", value_type);
            return FALSE;
    }

    if (!wintc_registry_ensure_proxy(registry, &local_error))
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    // Perform dbus call
    //
    gint      result        = 0;
    GVariant* value_variant = NULL;

    if (
        !zwin_registry_gdbus_call_get_key_value_sync(
            registry->proxy,
            key_path,
            value_name,
            value_type,
            &value_variant,
            &result,
            NULL,
            &local_error
        )
    )
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    if (result)
    {
        GVariant* inner = g_variant_get_variant(value_variant);

        switch (value_type)
        {
            case WINTC_REG_DWORD:
                *((gint*) value_data) =
                    g_variant_get_int32(inner);
                break;

            case WINTC_REG_QWORD:
                *((gint64*) value_data) =
                    g_variant_get_int64(inner);
                break;

            case WINTC_REG_SZ:
                *((gchar**) value_data) =
                    g_variant_dup_string(inner, NULL);
                break;

            default:
                g_critical("registry: unknown type %d", value_type);
                break;
        }

        g_variant_unref(inner);
    }

    g_variant_unref(value_variant);

    return TRUE;
}

gboolean wintc_registry_set_key_value(
    WinTCRegistry*         registry,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    const void*            value_data,
    gboolean               silent,
    GError**               error
)
{
    GError* local_error = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    if (!wintc_registry_ensure_proxy(registry, &local_error))
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    // Set up variant
    //
    GVariant* value_variant = NULL;

    switch (value_type)
    {
        case WINTC_REG_DWORD:
            value_variant =
                g_variant_new(
                    "v",
                    g_variant_new_int32(
                        *((const gint*) value_data)
                    )
                );
            break;

        case WINTC_REG_QWORD:
            value_variant =
                g_variant_new(
                    "v",
                    g_variant_new_int64(
                        *((const gint64*) value_data)
                    )
                );
            break;

        case WINTC_REG_SZ:
            value_variant =
                g_variant_new(
                    "v",
                    g_variant_new_string(
                        *((const gchar**) value_data)
                    )
                );
            break;

        default:
            g_critical("registry: unknown type %d", value_type);
            return FALSE;
    }

    // Perform dbus call
    //
    gint result = 0;

    if (
        !zwin_registry_gdbus_call_set_key_value_sync(
            registry->proxy,
            key_path,
            value_name,
            value_variant,
            silent,
            &result,
            NULL,
            &local_error
        )
    )
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    return !!result;
}

void wintc_registry_watch_key(
    WinTCRegistry*           registry,
    const gchar*             key_path,
    WinTCRegistryKeyCallback callback,
    gpointer                 user_data
)
{
    GError* error = NULL;

    if (!wintc_registry_ensure_proxy(registry, &error))
    {
        wintc_log_error_and_clear(&error);
        return;
    }

    // Add watcher
    //
    RegistryCBData* cb_data = g_new(RegistryCBData, 1);

    cb_data->callback  = callback;
    cb_data->user_data = user_data;

    if (!registry->connected_signal)
    {
        g_signal_connect(
            registry->proxy,
            "key-value-changed",
            G_CALLBACK(on_reg_key_value_changed),
            registry
        );
    }

    if (
        !g_hash_table_replace(
            registry->map_key_to_cb,
            g_strdup(key_path),
            cb_data
        )
    )
    {
        g_warning("registry: overwritten key watcher for %s", key_path);
    }
}

//
// PRIVATE FUNCTIONS
//
static gboolean wintc_registry_ensure_proxy(
    WinTCRegistry* registry,
    GError**       error
)
{
    GError* local_error = NULL;

    WINTC_SAFE_REF_CLEAR(error);

    if (registry->proxy)
    {
        return TRUE;
    }

    registry->proxy =
        zwin_registry_gdbus_proxy_new_for_bus_sync(
            G_BUS_TYPE_SESSION,
            G_DBUS_PROXY_FLAGS_NONE,
            "uk.oddmatics.wintc.registry",
            "/uk/oddmatics/wintc/registry/GDBUS",
            NULL,
            &local_error
        );

    if (!registry->proxy)
    {
        g_propagate_error(error, local_error);
        return FALSE;
    }

    return TRUE;
}

//
// CALLBACKS
//
static void on_reg_key_value_changed(
    WINTC_UNUSED(ZWinRegistryGDBUS* proxy),
    const gchar* key_path,
    const gchar* value_name,
    GVariant*    value_data,
    gpointer     user_data
)
{
    WinTCRegistry* registry = WINTC_REGISTRY(user_data);

    // Look up if there's a handler for this key
    //
    RegistryCBData* cb_data =
        g_hash_table_lookup(registry->map_key_to_cb, key_path);

    if (!cb_data)
    {
        return;
    }

    // Issue the callback
    //
    GVariant* inner = g_variant_get_variant(value_data);

    cb_data->callback(
         registry,
         key_path,
         value_name,
         inner,
         cb_data->user_data
    );

    g_variant_unref(inner);
}
