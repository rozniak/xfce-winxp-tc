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

//
// FORWARD DECLARATIONS
//
static WinTCIShextView* factory_view_by_guid_cb(
    WinTCShextViewAssoc assoc,
    const gchar*        assoc_str,
    const gchar*        url
);
static WinTCIShextView* factory_view_for_filesystem(
    WinTCShextViewAssoc assoc,
    const gchar*        assoc_str,
    const gchar*        url
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
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    const gchar* assoc_str,
    WINTC_UNUSED(const gchar*        url)
)
{
    WINTC_LOG_DEBUG("shell: create new shell view for %s", assoc_str);

    // Attempt to match one of the GUIDs we installed
    //
    if (g_ascii_strcasecmp(assoc_str, WINTC_SH_GUID_CPL) == 0)
    {
        return wintc_sh_view_cpl_new();
    }
    else if (g_ascii_strcasecmp(assoc_str, WINTC_SH_GUID_DESKTOP) == 0)
    {
        return wintc_sh_view_desktop_new();
    }
    else if (g_ascii_strcasecmp(assoc_str, WINTC_SH_GUID_DRIVES) == 0)
    {
        return wintc_sh_view_drives_new();
    }

    g_critical("shell: no view for GUID %s", assoc_str);

    return NULL;
}

static WinTCIShextView* factory_view_for_filesystem(
    WINTC_UNUSED(WinTCShextViewAssoc assoc),
    WINTC_UNUSED(const gchar*        assoc_str),
    const gchar* url
)
{
    const gchar* path_in_url = url + g_utf8_strlen("file://", -1);

    WINTC_LOG_DEBUG("shell: create new fs view for %s", url);

    return wintc_sh_view_fs_new(path_in_url);
}
