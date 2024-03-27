#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/path.h"
#include "../public/places.h"

#define SH_NUM_PLACES (WINTC_SH_PLACE_PRINTERS + 1)

//
// STATIC DATA
//
static gchar* s_place_paths[SH_NUM_PLACES] = { 0 };

//
// FORWARD DECLARATIONS
//
gchar* use_file_uri(
    const gchar* path
);

//
// PUBLIC FUNCTIONS
//
const gchar* wintc_sh_get_place_path(
    WinTCShPlace place
)
{
    if (!s_place_paths[place])
    {
        gchar* path = NULL;

        switch (place)
        {
            case WINTC_SH_PLACE_DESKTOP:
                path = wintc_sh_path_for_guid(WINTC_SH_GUID_DESKTOP);
                break;

            case WINTC_SH_PLACE_DOWNLOADS:
                path =
                    use_file_uri(
                        g_get_user_special_dir(G_USER_DIRECTORY_DOWNLOAD)
                    );
                break;

            case WINTC_SH_PLACE_DOCUMENTS:
                path =
                    use_file_uri(
                        g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS)
                    );
                break;

            case WINTC_SH_PLACE_MUSIC:
                path =
                    use_file_uri(
                        g_get_user_special_dir(G_USER_DIRECTORY_MUSIC)
                    );
                break;

            case WINTC_SH_PLACE_PICTURES:
                path =
                    use_file_uri(
                        g_get_user_special_dir(G_USER_DIRECTORY_PICTURES)
                    );
                break;

            case WINTC_SH_PLACE_VIDEO:
                path =
                    use_file_uri(
                        g_get_user_special_dir(G_USER_DIRECTORY_VIDEOS)
                    );
                break;

            case WINTC_SH_PLACE_DRIVES:
                path = wintc_sh_path_for_guid(WINTC_SH_GUID_DRIVES);
                break;

            case WINTC_SH_PLACE_CONTROLPANEL:
                path = wintc_sh_path_for_guid(WINTC_SH_GUID_CPL);
                break;

            case WINTC_SH_PLACE_PRINTERS:
                path = wintc_sh_path_for_guid(WINTC_SH_GUID_PRINTERS);
                break;

            default:
                g_critical("shcommon: place not implemented %d", place);
                break;
        }

        s_place_paths[place] = path;
    }

    return s_place_paths[place];
}

gchar* wintc_sh_path_for_guid(
    const gchar* guid
)
{
    return g_strdup_printf("::{%s}", guid);
}

//
// PRIVATE FUNCTIONS
//
gchar* use_file_uri(
    const gchar* path
)
{
    return g_strdup_printf("file://%s", path);
}
