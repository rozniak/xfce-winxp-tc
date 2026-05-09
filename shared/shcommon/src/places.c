#include <glib.h>

#include "../public/places.h"

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_SH_GUID_CPL        = "38f0b859-bfe8-49fe-88db-024d9c450147";
const gchar* WINTC_SH_GUID_DESKTOP    = "31591513-5e78-42f8-80d6-fd47629984d4";
const gchar* WINTC_SH_GUID_DRIVES     = "170f6110-d099-4fca-9307-fc1dc43eb71b";
const gchar* WINTC_SH_GUID_PRINTERS   = "2ea9a9c1-bea4-41aa-a263-7fca5e22ced0";
const gchar* WINTC_SH_GUID_RECYCLEBIN = "9eab79cf-8379-40be-b062-9fe3059f14da";

//
// PUBLIC FUNCTIONS
//
WinTCShPlace wintc_sh_get_place_from_guid(
    const gchar* guid
)
{
    static GHashTable* s_map_guid_to_place = NULL;

    gchar* guid_u = g_ascii_strup(guid, -1);

    if (!s_map_guid_to_place)
    {
        s_map_guid_to_place =
            g_hash_table_new(
                g_str_hash,
                g_str_equal
            );

        g_hash_table_insert(
            s_map_guid_to_place,
            g_ascii_strup(WINTC_SH_GUID_CPL, -1),
            GINT_TO_POINTER(WINTC_SH_PLACE_CONTROLPANEL)
        );
        g_hash_table_insert(
            s_map_guid_to_place,
            g_ascii_strup(WINTC_SH_GUID_DESKTOP, -1),
            GINT_TO_POINTER(WINTC_SH_PLACE_DESKTOP)
        );
        g_hash_table_insert(
            s_map_guid_to_place,
            g_ascii_strup(WINTC_SH_GUID_DRIVES, -1),
            GINT_TO_POINTER(WINTC_SH_PLACE_DRIVES)
        );
        g_hash_table_insert(
            s_map_guid_to_place,
            g_ascii_strup(WINTC_SH_GUID_PRINTERS, -1),
            GINT_TO_POINTER(WINTC_SH_PLACE_PRINTERS)
        );
        g_hash_table_insert(
            s_map_guid_to_place,
            g_ascii_strup(WINTC_SH_GUID_RECYCLEBIN, -1),
            GINT_TO_POINTER(WINTC_SH_PLACE_RECYCLEBIN)
        );
    }

    gpointer place =
        g_hash_table_lookup(s_map_guid_to_place, guid_u);

    g_free(guid_u);

    if (place)
    {
        return (WinTCShPlace) GPOINTER_TO_INT(place);
    }

    return WINTC_SH_PLACE_UNKNOWN;
}
