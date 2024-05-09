#ifndef __REGISTRY_VARIANT_H__
#define __REGISTRY_VARIANT_H__

#include <glib.h>

#include "regwrap.h"

//
// PUBLIC FUNCTIONS
//
WinTCRegistryValueType wintc_registry_get_type_for_variant(
    GVariant* variant
);

#endif
