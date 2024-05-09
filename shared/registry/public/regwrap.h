#ifndef __REGISTRY_REGWRAP_H__
#define __REGISTRY_REGWRAP_H__

#include <glib.h>

//
// PUBLIC ENUMS
//
typedef enum
{
    WINTC_REG_INVALID,
    WINTC_REG_DWORD,
    WINTC_REG_QWORD,
    WINTC_REG_SZ
} WinTCRegistryValueType;

//
// GLIB OOP BOILERPLATE
//
typedef struct _WinTCRegistryClass WinTCRegistryClass;
typedef struct _WinTCRegistry      WinTCRegistry;

#define WINTC_TYPE_REGISTRY            (wintc_registry_get_type())
#define WINTC_REGISTRY(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_REGISTRY, WinTCRegistry))
#define WINTC_REGISTRY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_REGISTRY, WinTCRegistryClass))
#define IS_WINTC_REGISTRY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_REGISTRY))
#define IS_WINTC_REGISTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_REGISTRY))
#define WINTC_REGISTRY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_REGISTRY, WinTCRegistry))

GType wintc_registry_get_type(void) G_GNUC_CONST;

//
// CALLBACK PROTOTYPES
//
typedef void (*WinTCRegistryKeyCallback) (
    WinTCRegistry*         registry,
    const gchar*           key_path,
    const gchar*           value_name,
    GVariant*              value_variant,
    gpointer               user_data
);

//
// PUBLIC FUNCTIONS
//
WinTCRegistry* wintc_registry_new(void);

gboolean wintc_registry_create_key(
    WinTCRegistry* registry,
    const gchar*   key_path,
    GError**       error
);

gboolean wintc_registry_get_key_value(
    WinTCRegistry*         registry,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data,
    GError**               error
);

gboolean wintc_registry_set_key_value(
    WinTCRegistry*         registry,
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    const void*            value_data,
    gboolean               silent,
    GError**               error
);

void wintc_registry_watch_key(
    WinTCRegistry*           registry,
    const gchar*             key_path,
    WinTCRegistryKeyCallback callback,
    gpointer                 user_data
);

#endif
