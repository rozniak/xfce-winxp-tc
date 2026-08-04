#include <gio/gunixmounts.h>
#include <glib.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../public/drvmon.h"
#include "../public/nmspace.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_NULL,
    PROP_SHEXT_HOST,
    N_PROPERTIES
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCShellDrive
{
    GDrive*         drive;
    WinTCShextHost* shext_host;
    const gchar*    guid_category;

    GList*      list_unix_paths;
    GList*      list_volumes;
    GHashTable* map_id_to_icon;
} WinTCShellDrive;

//
// FORWARD DECLARATIONS
//
static void wintc_sh_drive_monitor_constructed(
    GObject* object
);
static void wintc_sh_drive_monitor_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_shell_drive_add_icon(
    WinTCShellDrive*           sh_drive,
    gchar*                     ident,
    gchar*                     icon_name,
    gpointer                   priv,
    WinTCShextActivateItemFunc activate_cb
);
static WinTCShellDrive* wintc_shell_drive_new(
    GDrive*         drive,
    WinTCShextHost* shext_host
);

static void clear_view_item(
    WinTCShextViewItem* item
);

static gboolean cb_shext_activate_item_drive(
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);

static WinTCShellDrive* cb_vm_drive_init(
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
// STATIC DATA
//
static GHashTable* S_MAP_SHEXT_HOST_TO_DRVMON = NULL;

static GParamSpec* wintc_sh_drive_monitor_properties[N_PROPERTIES] = { 0 };

//
// GLIB OOP/CLASS INSTANCE DEFINITIONS
//
struct _WinTCShDriveMonitorClass
{
    GObjectClass __parent__;
};

struct _WinTCShDriveMonitor
{
    GObject __parent__;

    // State
    //
    WinTCShextHost* shext_host;
};

//
// GLIB TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShDriveMonitor,
    wintc_sh_drive_monitor,
    G_TYPE_OBJECT
)

static void wintc_sh_drive_monitor_class_init(
    WinTCShDriveMonitorClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_drive_monitor_constructed;
    object_class->set_property = wintc_sh_drive_monitor_set_property;

    wintc_sh_drive_monitor_properties[PROP_SHEXT_HOST] =
        g_param_spec_object(
            "shext-host",
            "ShextHost",
            "The shell extension host.",
            WINTC_TYPE_SHEXT_HOST,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        );

    g_object_class_install_properties(
        object_class,
        N_PROPERTIES,
        wintc_sh_drive_monitor_properties
    );

    // Init global map if necessary
    //
    if (!S_MAP_SHEXT_HOST_TO_DRVMON)
    {
        S_MAP_SHEXT_HOST_TO_DRVMON =
            g_hash_table_new(g_direct_hash, g_direct_equal);
    }
}

static void wintc_sh_drive_monitor_init(
    WINTC_UNUSED(WinTCShDriveMonitor* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_drive_monitor_constructed(
    GObject* object
)
{
    WinTCShDriveMonitor* drvmon = WINTC_SH_DRIVE_MONITOR(object);

    // Grab volume monitor
    //
    static GVolumeMonitor* s_monitor = NULL;

    if (!s_monitor)
    {
        s_monitor = g_volume_monitor_get();
    }

    // Establish volume monitor signals
    //
    GList* drives    = g_volume_monitor_get_connected_drives(s_monitor);
    GList* sh_drives = NULL;

    for (GList* iter = drives; iter; iter = iter->next)
    {
        sh_drives =
            g_list_prepend(
                sh_drives,
                cb_vm_drive_init(
                    G_DRIVE(iter->data),
                    drvmon->shext_host
                )
            );
    }

    g_signal_connect(
        s_monitor,
        "drive-connected",
        G_CALLBACK(on_volume_monitor_drive_connected),
        drvmon->shext_host
    );

    // Scan for UNIX mounts that the volume watcher hides from us
    //
    GList* unix_mounts = g_unix_mount_entries_get(NULL);

    for (GList* iter = unix_mounts; iter; iter = iter->next)
    {
        GUnixMountEntry* mount = (GUnixMountEntry*) iter->data;

        if (
            !g_unix_mount_entry_is_system_internal(mount) ||
            g_strcmp0(g_unix_mount_entry_get_mount_path(mount), "/") != 0
        )
        {
            continue;
        }

        // Attempt to find the drive that should be mapped with this
        //
        for (GList* iter2 = sh_drives; iter2; iter2 = iter2->next)
        {
            WinTCShellDrive* sh_drive = (WinTCShellDrive*) iter2->data;

            gchar* ident =
                g_drive_get_identifier(
                    sh_drive->drive,
                    G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
                );

            if (
                g_str_has_prefix(
                    g_unix_mount_entry_get_device_path(mount),
                    ident
                )
            )
            {
                WINTC_LOG_DEBUG(
                    "shell: drvmon: determined unix path %s mapped to %s",
                    g_unix_mount_entry_get_mount_path(mount),
                    g_unix_mount_entry_get_device_path(mount)
                );

                sh_drive->list_unix_paths =
                    g_list_append(
                        sh_drive->list_unix_paths,
                        g_strdup(g_unix_mount_entry_get_device_path(mount))
                    );

                // We know this is a fixed path
                //
                wintc_shell_drive_add_icon(
                    sh_drive,
                    g_strdup(g_unix_mount_entry_get_mount_path(mount)),
                    g_strdup("drive-harddisk"),
                    g_strdup(g_unix_mount_entry_get_mount_path(mount)),
                    (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
                );
            }

            g_free(ident);
        }
    }

    g_list_free_full(unix_mounts, (GDestroyNotify) g_unix_mount_entry_free);
    g_list_free_full(drives,      (GDestroyNotify) g_object_unref);
    g_list_free(sh_drives);
}

static void wintc_sh_drive_monitor_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
)
{
    WinTCShDriveMonitor* drvmon = WINTC_SH_DRIVE_MONITOR(object);

    switch (prop_id)
    {
        case PROP_SHEXT_HOST:
            //
            // FIXME: Take out weak ref here
            //
            drvmon->shext_host = g_value_get_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShDriveMonitor* wintc_sh_drive_monitor_get(
    WinTCShextHost* shext_host
)
{
    WinTCShDriveMonitor* drvmon =
        g_hash_table_lookup(S_MAP_SHEXT_HOST_TO_DRVMON, shext_host);

    if (drvmon)
    {
        return drvmon;
    }

    // Need a new one
    //
    drvmon =
        g_object_new(
            WINTC_TYPE_SH_DRIVE_MONITOR,
            "shext-host", shext_host,
            NULL
        );

    g_hash_table_insert(
        S_MAP_SHEXT_HOST_TO_DRVMON,
        shext_host,
        drvmon
    );

    return drvmon;
}

//
// PRIVATE FUNCTIONS
//
static void wintc_shell_drive_add_icon(
    WinTCShellDrive*           sh_drive,
    gchar*                     ident,
    gchar*                     icon_name,
    gpointer                   priv,
    WinTCShextActivateItemFunc activate_cb
)
{
    gchar*              id   = g_strconcat("nmspace", ident, NULL);
    WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

    item->display_name = ident;
    item->icon_name    = icon_name;
    item->is_leaf      = FALSE;
    item->hash         = g_str_hash(id);
    item->hint         = 0;
    item->priv         = priv;

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
        activate_cb,
        NULL
    );

    g_free(id);
}

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

    sh_drive->drive           = drive;
    sh_drive->shext_host      = shext_host;
    sh_drive->list_volumes    = NULL;
    sh_drive->list_unix_paths = NULL;

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
    g_free((gchar*) item->priv);
    g_free(item);
}

//
// CALLBACKS
//
static gboolean cb_shext_activate_item_drive(
    WINTC_UNUSED(WinTCShextHost* shext_host),
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    WINTC_UNUSED(GError**        error)
)
{
    if (item->priv)
    {
        path_info->base_path =
            g_strdup_printf("file://%s", ((gchar*) item->priv));
    }
    else
    {
        g_warning("shell: no path for %s", item->display_name);
    }

    return TRUE;
}

static WinTCShellDrive* cb_vm_drive_init(
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

    return sh_drive;
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
            //
            // FIXME: This should have its own callback for prompting for media
            //
            wintc_shell_drive_add_icon(
                sh_drive,
                g_steal_pointer(&ident),
                wintc_icon_get_available_name(
                    g_drive_get_icon(sh_drive->drive)
                ),
                NULL,
                (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
            );
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
            GFile* root = g_mount_get_root(mount);

            wintc_shell_drive_add_icon(
                sh_drive,
                g_steal_pointer(&ident),
                wintc_icon_get_available_name(
                    g_mount_get_icon(mount)
                ),
                g_file_get_path(root),
                (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
            );

            g_object_unref(root);
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
