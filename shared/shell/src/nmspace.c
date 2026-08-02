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
// PRIVATE STRUCTURES
//
typedef struct _WinTCShellDrive
{
    GDrive*         drive;
    WinTCShextHost* shext_host;
    const gchar*    guid_category;

    GList*      list_volumes;
    GHashTable* map_id_to_icon;
} WinTCShellDrive;

//
// FORWARD DECLARATIONS
//
static WinTCShellDrive* wintc_shell_drive_new(
    GDrive*         drive,
    WinTCShextHost* shext_host
);

static void clear_view_item(
    WinTCShextViewItem* item
);

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

static gboolean cb_shext_activate_item_drive(
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static void cb_vm_drive_init(
    GDrive*         drive,
    WinTCShextHost* shext_host
);
static void cb_vm_drive_update_state(
    WinTCShellDrive* sh_drive
);
static gboolean cb_vm_volume_update_state(
    WinTCShellDrive* sh_drive,
    GVolume*         volume
);

static void on_drive_changed(
    GDrive*         drive,
    gpointer        user_data
);
static void on_drive_disconnected(
    GDrive*         drive,
    gpointer        user_data
);
static void on_volume_monitor_drive_connected(
    GVolumeMonitor* self,
    GDrive*         drive,
    gpointer        user_data
);

//
// PUBLIC CONSTANTS
//
const gchar* WINTC_SH_GUID_CATEGORY_DRIVES =
    "4cd9ad86-1950-4412-a511-25d92d085519";
const gchar* WINTC_SH_GUID_CATEGORY_LOCAL_FILES =
    "3e93a6c1-efc5-435f-996f-5e0eb97e92a5";
const gchar* WINTC_SH_GUID_CATEGORY_REMOVABLES =
    "5cb97e0a-00ca-448c-8c89-6e79892ef3bb";
const gchar* WINTC_SH_GUID_CATEGORY_OTHER =
    "a388a67e-bdaf-47bf-844f-c68549ff574b";

//
// PUBLIC FUNCTIONS
//
gboolean wintc_sh_init_builtin_extensions(
    WinTCShextHost* shext_host
)
{
    static GVolumeMonitor* s_monitor = NULL;

    if (!s_monitor)
    {
        s_monitor = g_volume_monitor_get();
    }

    // Register toplevels
    //
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_toplevel_category(
            shext_host,
            WINTC_SH_GUID_CATEGORY_DRIVES,
            "Hard Disk Drives"
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_toplevel_category(
            shext_host,
            WINTC_SH_GUID_CATEGORY_LOCAL_FILES,
            "Files Stored on This Computer"
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_toplevel_category(
            shext_host,
            WINTC_SH_GUID_CATEGORY_REMOVABLES,
            "Devices with Removable Storage"
        ),
        FALSE
    );
    WINTC_RETURN_VAL_IF_FAIL(
        wintc_shext_host_register_toplevel_category(
            shext_host,
            WINTC_SH_GUID_CATEGORY_OTHER,
            "Other"
        ),
        FALSE
    );

    // Establish volume monitor signals
    //
    GList* drives = g_volume_monitor_get_connected_drives(s_monitor);

    for (GList* iter = drives; iter; iter = iter->next)
    {
        cb_vm_drive_init(
            G_DRIVE(iter->data),
            shext_host
        );
    }

    g_list_free_full(drives, (GDestroyNotify) g_object_unref);

    g_signal_connect(
        s_monitor,
        "drive-connected",
        G_CALLBACK(on_volume_monitor_drive_connected),
        shext_host
    );

    // Register views
    //
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
// PRIVATE FUNCTIONS
//
static WinTCShellDrive* wintc_shell_drive_new(
    GDrive*         drive,
    WinTCShextHost* shext_host
)
{
    WinTCShellDrive* sh_drive = g_new(WinTCShellDrive, 1);

    //
    // FIXME: Potentially need to soft ref shext host to destroy ourselves +
    //        the signals attached to GDrive?
    //

    sh_drive->drive         = drive;
    sh_drive->shext_host    = shext_host;
    sh_drive->list_volumes  = NULL;

    sh_drive->map_id_to_icon =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            (GDestroyNotify) clear_view_item
        );

    if (
        g_drive_is_media_removable(drive) ||
        g_drive_is_removable(drive)
    )
    {
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_REMOVABLES;
    }
    else
    {
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_DRIVES;
    }

    return sh_drive;
}

static void clear_view_item(
    WinTCShextViewItem* item
)
{
    g_free(item->display_name);
    g_free(item->icon_name);
    g_free(item);
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
            return wintc_sh_view_drives_new(shext_host);

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

static gboolean cb_shext_activate_item_drive(
    WINTC_UNUSED(WinTCShextHost*     shext_host),
    WinTCShextViewItem* item,
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    WINTC_UNUSED(GError**            error)
)
{
    // FIXME: Implement this
    //
    g_message("Success! Activated %s", item->display_name);

    return TRUE;
}

static void cb_vm_drive_init(
    GDrive*         drive,
    WinTCShextHost* shext_host
)
{
    gchar* name  = g_drive_get_name(drive);
    gchar* ident = g_drive_get_identifier(
                       drive,
                       G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
                   );

    WINTC_LOG_DEBUG("shell: drvmon: init drive %s (id: %s)", name, ident);

    g_free(name);
    g_free(ident);

    // Set up drive
    //
    WinTCShellDrive* sh_drive =
        wintc_shell_drive_new(
            drive,
            shext_host
        );

    cb_vm_drive_update_state(sh_drive);

    g_signal_connect(
        drive,
        "changed",
        G_CALLBACK(on_drive_changed),
        sh_drive
    );
    g_signal_connect(
        drive,
        "disconnected",
        G_CALLBACK(on_drive_disconnected),
        sh_drive
    );
}

static void cb_vm_drive_update_state(
    WinTCShellDrive* sh_drive
)
{
    gchar* ident =
        g_drive_get_identifier(
            sh_drive->drive,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    // If there are no volumes for this drive, then we need to clear its icons
    // UNLESS it is a drive with removable media OR is removable itself (could
    // be an unformatted USB or something)
    //
    if (!g_drive_has_volumes(sh_drive->drive))
    {
        gboolean requires_placeholder =
            g_drive_is_media_removable(sh_drive->drive) ||
            g_drive_is_removable(sh_drive->drive);

        GHashTableIter iter;
        const gchar*   id_iter;

        WINTC_LOG_DEBUG("shell: drvmon: no volumes for drive %s", ident);

        g_hash_table_iter_init(&iter, sh_drive->map_id_to_icon);

        while (g_hash_table_iter_next(&iter, (void**) &id_iter, NULL))
        {
            wintc_shext_host_remove_toplevel_item(
                sh_drive->shext_host,
                sh_drive->guid_category,
                id_iter
            );

            g_hash_table_iter_remove(&iter);
        }

        if (requires_placeholder)
        {
            gchar*              id   = g_strconcat("nmspace", ident, NULL);
            WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

            item->display_name = ident;
            item->icon_name    = wintc_icon_get_available_name(
                                     g_drive_get_icon(sh_drive->drive)
                                 );
            item->is_leaf      = FALSE;
            item->hash         = g_str_hash(id);
            item->hint         = 0;
            item->priv         = NULL;

            g_hash_table_insert(
                sh_drive->map_id_to_icon,
                ident,
                item
            );

            wintc_shext_host_add_toplevel_item(
                sh_drive->shext_host,
                sh_drive->guid_category,
                ident,
                item,
                (WinTCShextActivateItemFunc) cb_shext_activate_item_drive,
                NULL
            );

            g_free(id);
            ident = NULL; // We've stolen it
        }

        goto cleanup;
    }

    // Collect up any volumes for the drive
    //
    GList* volumes = g_drive_get_volumes(sh_drive->drive);

    for (GList* iter = volumes; iter; iter = iter->next)
    {
        GVolume* volume = (GVolume*) iter->data;

        if (!cb_vm_volume_update_state(sh_drive, volume))
        {
            g_object_unref(volume);
        }
    }

    g_list_free(volumes);

cleanup:
    g_free(ident);
}

static gboolean cb_vm_volume_update_state(
    WinTCShellDrive* sh_drive,
    GVolume*         volume
)
{
    gboolean ref_transferred = FALSE;

    // Is this a new volume?
    //
    if (!g_list_find(sh_drive->list_volumes, volume))
    {
        WINTC_LOG_DEBUG("shell: drvmon: new volume...");

        sh_drive->list_volumes =
            g_list_append(sh_drive->list_volumes, volume);

        //
        // FIXME: Connect signals
        //

        ref_transferred = TRUE;
    }

    // Does it have a mount point?
    //
    gchar*  ident = g_volume_get_identifier(
                        volume,
                        G_VOLUME_IDENTIFIER_KIND_UNIX_DEVICE
                    );
    GMount* mount = g_volume_get_mount(volume);

    WINTC_LOG_DEBUG("shell: drvmon: examining volume %s", ident);

    if (mount)
    {
        if (!g_hash_table_lookup(sh_drive->map_id_to_icon, ident))
        {
            gchar*              id   = g_strconcat("nmspace", ident, NULL);
            WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

            item->display_name = ident;
            item->icon_name    = wintc_icon_get_available_name(
                                     g_mount_get_icon(mount)
                                 );
            item->is_leaf      = FALSE;
            item->hash         = g_str_hash(id);
            item->hint         = 0;
            item->priv         = NULL;

            g_hash_table_insert(
                sh_drive->map_id_to_icon,
                ident,
                item
            );

            wintc_shext_host_add_toplevel_item(
                sh_drive->shext_host,
                sh_drive->guid_category,
                ident,
                item,
                (WinTCShextActivateItemFunc) cb_shext_activate_item_drive,
                NULL
            );

            g_free(id);
            ident = NULL; // We've stolen it
        }
    }
    else
    {
        if (g_hash_table_remove(sh_drive->map_id_to_icon, ident))
        {
            wintc_shext_host_remove_toplevel_item(
                sh_drive->shext_host,
                sh_drive->guid_category,
                ident
            );
        }
    }

    g_free(ident);

    return ref_transferred;
}

static void on_drive_changed(
    GDrive* drive,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG(
        "Drive just changed: %s",
        g_drive_get_name(drive)
    );
}

static void on_drive_disconnected(
    GDrive* drive,
    WINTC_UNUSED(gpointer user_data)
)
{
    WINTC_LOG_DEBUG(
        "Drive disconnected: %s",
        g_drive_get_name(drive)
    );
}

static void on_volume_monitor_drive_connected(
    WINTC_UNUSED(GVolumeMonitor* self),
    GDrive*  drive,
    gpointer user_data
)
{
    cb_vm_drive_init(drive, WINTC_SHEXT_HOST(user_data));
}
