#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>

#include "../public/nmspace.h"
#include "../public/vwcpl.h"
#include "../public/vwdesk.h"
#include "../public/vwdrives.h"
#include "../public/vwfs.h"
#include "../public/vwtrash.h"

//
// FORWARD DECLARATIONS
//
static WinTCIShextView* factory_view_by_guid_cb(
    WinTCShextHost*           shext_host,
    WinTCShextViewAssoc       assoc,
    const gchar*              assoc_str,
    const WinTCShextPathInfo* path_info
);
static WinTCIShextView* factory_view_for_filesystem(
    WinTCShextHost*           shext_host,
    WinTCShextViewAssoc       assoc,
    const gchar*              assoc_str,
    const WinTCShextPathInfo* path_info
);

//
// PUBLIC FUNCTIONS
//
gboolean wintc_sh_init_builtin_extensions(
    WinTCShextHost* shext_host
)
{
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_view(
            shext_host,
            WINTC_SH_GUID_CPL,
            factory_view_by_guid_cb
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_view(
            shext_host,
            WINTC_SH_GUID_DESKTOP,
            factory_view_by_guid_cb
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_view(
            shext_host,
            WINTC_SH_GUID_DRIVES,
            factory_view_by_guid_cb
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_view(
            shext_host,
            WINTC_SH_GUID_RECYCLEBIN,
            factory_view_by_guid_cb
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_use_view_for_mime(
            shext_host,
            "x-scheme-handler/file",
            factory_view_for_filesystem
        ),
        FALSE
    );

    return TRUE;
}

void wintc_sh_init_namespace_tree(
    WINTC_UNUSED(GtkTreeModel*   tree_model),
    WINTC_UNUSED(WinTCShextHost* shext_host)
)
{
    g_critical("%s Not Implemented", __func__);
}

//
// CALLBACKS
//
static WinTCIShextView* factory_view_by_guid_cb(
    WinTCShextHost*           shext_host,
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    const gchar*              assoc_str,
    const WinTCShextPathInfo* path_info
)
{
    WINTC_LOG_DEBUG("shell: create new shell view for %s", assoc_str);

    WinTCShPlace place = wintc_sh_get_place_from_guid(assoc_str);

    switch (place)
    {
        case WINTC_SH_PLACE_CONTROLPANEL:
            return wintc_sh_view_cpl_new();

        case WINTC_SH_PLACE_DESKTOP:
            // If there is an extended path, then forward onto the FS view
            //
            if (path_info->extended_path)
            {
                return wintc_sh_view_fs_new(shext_host, path_info);
            }

            return wintc_sh_view_desktop_new(shext_host);

        case WINTC_SH_PLACE_DRIVES:
            return wintc_sh_view_drives_new();

        case WINTC_SH_PLACE_RECYCLEBIN:
            return wintc_sh_view_trash_new();

        default:
            g_critical("shell: no view for GUID %s", assoc_str);
            return NULL;
    }
}

static WinTCIShextView* factory_view_for_filesystem(
    WinTCShextHost* shext_host,
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    WINTC_UNUSED(const gchar*        assoc_str),
    const WinTCShextPathInfo* path_info
)
{
    WINTC_LOG_DEBUG(
        "shell: create new fs view for %s",
        path_info->base_path
    );

    return wintc_sh_view_fs_new(shext_host, path_info);
}
