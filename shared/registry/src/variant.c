#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/regwrap.h"
#include "../public/variant.h"

//
// PUBLIC FUNCTIONS
//
WinTCRegistryValueType wintc_registry_get_type_for_variant(
    GVariant* variant
)
{
    if (g_variant_is_of_type(variant, G_VARIANT_TYPE_INT32))
    {
        return WINTC_REG_DWORD;
    }
    else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_INT64))
    {
        return WINTC_REG_QWORD;
    }
    else if (g_variant_is_of_type(variant, G_VARIANT_TYPE_STRING))
    {
        return WINTC_REG_SZ;
    }

    return WINTC_REG_INVALID;
}
