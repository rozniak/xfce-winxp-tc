#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <glib.h>
#include <wintc/registry.h>

//
// INTERNAL FUNCTIONS
//
void backend_close(void);
gboolean backend_init(void);

gboolean backend_create_key(
    const gchar* key_path
);
gboolean backend_get_key_value(
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data
);
gboolean backend_set_key_value(
    const gchar*           key_path,
    const gchar*           value_name,
    WinTCRegistryValueType value_type,
    void*                  value_data
);

#endif
