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

enum
{
    WINTC_SH_DRIVE_TYPE_UNKNOWN,
    WINTC_SH_DRIVE_TYPE_FIXED,
    WINTC_SH_DRIVE_TYPE_REMOVABLE,
    WINTC_SH_DRIVE_TYPE_MEDIA_CONTAINER
};

//
// PRIVATE STRUCTURES
//
typedef struct _WinTCShellDrive
{
    WinTCShDriveMonitor* drvmon;

    GDrive*         drive;
    const gchar*    guid_category;
    gint            drive_type;

    GList*      list_unix_paths;
} WinTCShellDrive;

//
// FORWARD DECLARATIONS
//
static void wintc_sh_drive_monitor_constructed(
    GObject* object
);
static void wintc_sh_drive_monitor_dispose(
    GObject* object
);
static void wintc_sh_drive_monitor_set_property(
    GObject*      object,
    guint         prop_id,
    const GValue* value,
    GParamSpec*   pspec
);

static void wintc_sh_drive_monitor_add_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
);
static void wintc_sh_drive_monitor_add_icon(
    WinTCShDriveMonitor*       drvmon,
    const gchar*               obj_path,
    const gchar*               guid_category,
    gchar*                     display_name,
    gchar*                     icon_name,
    gchar*                     priv,
    WinTCShextActivateItemFunc activate_cb
);
static void wintc_sh_drive_monitor_add_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
);
static void wintc_sh_drive_monitor_add_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
);

static WinTCShellDrive* wintc_sh_drive_monitor_get_shell_drive(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount,
    GVolume*             volume
);

static void wintc_sh_drive_monitor_register_drive_icon(
    WinTCShDriveMonitor* drvmon,
    WinTCShellDrive*     sh_drive
);

static void wintc_sh_drive_monitor_remove_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
);
static void wintc_sh_drive_monitor_remove_icon(
    WinTCShDriveMonitor* drvmon,
    const gchar*         obj_path,
    const gchar*         guid_category
);
static void wintc_sh_drive_monitor_remove_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
);
static void wintc_sh_drive_monitor_remove_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
);

static void wintc_shell_drive_free(
    WinTCShellDrive* sh_drive
);
static void wintc_shell_drive_true_obj_path(
    gchar** obj_path
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

static void on_volume_monitor_drive_connected(
    GVolumeMonitor* self,
    GDrive*         drive,
    gpointer        user_data
);
static void on_volume_monitor_drive_disconnected(
    GVolumeMonitor* self,
    GDrive*         drive,
    gpointer        user_data
);
static void on_volume_monitor_mount_added(
    GVolumeMonitor* self,
    GMount*         mount,
    gpointer        user_data
);
static void on_volume_monitor_mount_removed(
    GVolumeMonitor* self,
    GMount*         mount,
    gpointer        user_data
);
static void on_volume_monitor_volume_added(
    GVolumeMonitor* self,
    GVolume*        volume,
    gpointer        user_data
);
static void on_volume_monitor_volume_removed(
    GVolumeMonitor* self,
    GVolume*        volume,
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

    GHashTable* map_drive_to_sh_drive;
    GHashTable* map_id_to_icon;
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
    object_class->dispose      = wintc_sh_drive_monitor_dispose;
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
    WinTCShDriveMonitor* self
)
{
    self->map_drive_to_sh_drive =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) wintc_shell_drive_free
        );
    self->map_id_to_icon =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            (GDestroyNotify) clear_view_item
        );
}

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

    // Enum drives
    //
    GList* drives = g_volume_monitor_get_connected_drives(s_monitor);

    for (GList* iter = drives; iter; iter = iter->next)
    {
        wintc_sh_drive_monitor_add_drive(drvmon, G_DRIVE(iter->data));
    }

    g_list_free_full(drives, (GDestroyNotify) g_object_unref);

    // Enum volumes
    //
    GList* volumes = g_volume_monitor_get_volumes(s_monitor);

    for (GList* iter = volumes; iter; iter = iter->next)
    {
        wintc_sh_drive_monitor_add_volume(drvmon, G_VOLUME(iter->data));
    }

    g_list_free_full(volumes, (GDestroyNotify) g_object_unref);

    // Enum mounts
    //
    GList* mounts = g_volume_monitor_get_mounts(s_monitor);

    for (GList* iter = mounts; iter; iter = iter->next)
    {
        wintc_sh_drive_monitor_add_mount(drvmon, G_MOUNT(iter->data));
    }

    g_list_free_full(mounts, (GDestroyNotify) g_object_unref);

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
        WinTCShellDrive* sh_drive;
        GHashTableIter   iter_ht;

        g_hash_table_iter_init(&iter_ht, drvmon->map_drive_to_sh_drive);

        while (g_hash_table_iter_next(&iter_ht, NULL, (void*) &sh_drive))
        {
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

                // Bin any existing drive icon
                //
                wintc_sh_drive_monitor_remove_drive(drvmon, sh_drive->drive);

                // We know this is a fixed path
                //
                const gchar* mount_path =
                    g_unix_mount_entry_get_mount_path(mount);

                wintc_sh_drive_monitor_add_icon(
                    drvmon,
                    mount_path,
                    sh_drive->guid_category,
                    g_strdup_printf(
                        "Local Disk (%s)",
                        mount_path
                    ),
                    g_strdup("drive-harddisk"),
                    g_strdup(mount_path),
                    (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
                );
            }

            g_free(ident);
        }
    }

    g_list_free_full(unix_mounts, (GDestroyNotify) g_unix_mount_entry_free);

    // Connect monitor signals
    //
    g_signal_connect(
        s_monitor,
        "drive-connected",
        G_CALLBACK(on_volume_monitor_drive_connected),
        drvmon
    );
    g_signal_connect(
        s_monitor,
        "drive-disconnected",
        G_CALLBACK(on_volume_monitor_drive_disconnected),
        drvmon
    );
    g_signal_connect(
        s_monitor,
        "mount-added",
        G_CALLBACK(on_volume_monitor_mount_added),
        drvmon
    );
    g_signal_connect(
        s_monitor,
        "mount-removed",
        G_CALLBACK(on_volume_monitor_mount_removed),
        drvmon
    );
    g_signal_connect(
        s_monitor,
        "volume-added",
        G_CALLBACK(on_volume_monitor_volume_added),
        drvmon
    );
    g_signal_connect(
        s_monitor,
        "volume-removed",
        G_CALLBACK(on_volume_monitor_volume_removed),
        drvmon
    );
}

static void wintc_sh_drive_monitor_dispose(
    GObject* object
)
{
    WinTCShDriveMonitor* drvmon = WINTC_SH_DRIVE_MONITOR(object);

    g_hash_table_unref(drvmon->map_id_to_icon);

    (G_OBJECT_CLASS(wintc_sh_drive_monitor_parent_class))->dispose(object);
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
static void wintc_sh_drive_monitor_add_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
)
{
    WinTCShellDrive* sh_drive = g_new(WinTCShellDrive, 1);

    sh_drive->drvmon          = drvmon;
    sh_drive->drive           = drive;
    sh_drive->list_unix_paths = NULL;

    wintc_sh_drive_monitor_register_drive_icon(drvmon, sh_drive);

    // Map drive now
    //
    g_hash_table_insert(
        drvmon->map_drive_to_sh_drive,
        drive,
        sh_drive
    );
}

static void wintc_sh_drive_monitor_add_icon(
    WinTCShDriveMonitor*       drvmon,
    const gchar*               obj_path,
    const gchar*               guid_category,
    gchar*                     display_name,
    gchar*                     icon_name,
    gchar*                     priv,
    WinTCShextActivateItemFunc activate_cb
)
{
    WinTCShextViewItem* item = g_new(WinTCShextViewItem, 1);

    item->display_name = display_name;
    item->icon_name    = icon_name;
    item->is_leaf      = FALSE;
    item->hash         = g_str_hash(obj_path);
    item->hint         = 0;
    item->priv         = priv;

    wintc_shext_host_add_toplevel_item(
        drvmon->shext_host,
        guid_category,
        obj_path,
        item,
        activate_cb,
        NULL // FIXME: Error handling
    );

    g_hash_table_insert(
        drvmon->map_id_to_icon,
        g_strdup(obj_path),
        item
    );
}

static void wintc_sh_drive_monitor_add_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
    gchar*   obj_path;
    GVolume* volume = g_mount_get_volume(mount);

    if (!volume)
    {
        //
        // FIXME: Handle mounts with no volume - likely network mounts?
        //
        g_warning("%s", "shell: drvmon: mounts w/ no volume unhandled");
        return;
    }

    // Look up owner shell drive
    //
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, mount, NULL);

    if (!sh_drive)
    {
        g_critical("%s", "shell: drvmon: mount with no existing drive!");
        goto cleanup;
    }

    // If the volume has an icon, remove it
    //
    obj_path =
        g_volume_get_identifier(
            volume,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category
    );

    g_free(obj_path);

    // Install mount icon
    //
    GFile* file = g_mount_get_default_location(mount);
    GIcon* icon = g_mount_get_icon(mount);

    obj_path = g_file_get_path(file);

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_add_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category,
        g_mount_get_name(mount),
        wintc_icon_get_available_name(icon),
        obj_path,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(file);
    g_object_unref(icon);

cleanup:
    g_object_unref(volume);
}

static void wintc_sh_drive_monitor_add_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
)
{
   gchar* obj_path = NULL;

    // Look up owner shell drive
    //
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, NULL, volume);

    if (!sh_drive)
    {
        g_critical("%s", "shell: drvmon: volume with no existing drive!");
        return;
    }

    // If the drive has an icon, remove it
    //
    obj_path =
        g_drive_get_identifier(
            sh_drive->drive,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category
    );

    g_free(obj_path);

    // Install placeholder volume icon
    //
    GIcon* icon = g_volume_get_icon(volume);

    obj_path =
        g_volume_get_identifier(
            volume,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_sh_drive_monitor_add_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category,
        g_volume_get_name(volume),
        wintc_icon_get_available_name(icon),
        obj_path,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(icon);
}

static WinTCShellDrive* wintc_sh_drive_monitor_get_shell_drive(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount,
    GVolume*             volume
)
{
    GDrive*          drive;
    WinTCShellDrive* sh_drive;

    drive =
        mount ? g_mount_get_drive(mount) : g_volume_get_drive(volume);

    sh_drive =
        g_hash_table_lookup(drvmon->map_drive_to_sh_drive, drive);

    g_object_unref(drive);

    return sh_drive;
}

static void wintc_sh_drive_monitor_register_drive_icon(
    WinTCShDriveMonitor* drvmon,
    WinTCShellDrive*     sh_drive
)
{
    // This should be a no-op if the drive has a UNIX mount, only the mount
    // needs to be shown and not the drive placeholder
    //
    if (sh_drive->list_unix_paths)
    {
        return;
    }

    // Determine drive type and category, insert default icon
    //
    GIcon* icon     = g_drive_get_icon(sh_drive->drive);
    gchar* obj_path = g_drive_get_identifier(
                          sh_drive->drive,
                          G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
                      );
    gchar* text;

    if (g_drive_is_media_removable(sh_drive->drive))
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_MEDIA_CONTAINER;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_REMOVABLES;

        text = g_strdup_printf("Media Drive (%s)", obj_path);

        //
        // FIXME: Activate func needs to be specific to media disks
        //

    }
    else if (g_drive_is_removable(sh_drive->drive))
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_REMOVABLE;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_REMOVABLES;

        text = g_strdup_printf("Removable Disk (%s)", obj_path);

        //
        // FIXME: Activate func probably needs to be format dialog
        //
    }
    else
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_FIXED;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_DRIVES;

        text = g_strdup_printf("Fixed Disk (%s)", obj_path);

        //
        // FIXME: Activate func probably needs to be format dialog
        //
    }

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_add_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category,
        text,
        wintc_icon_get_available_name(icon),
        obj_path,
       (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(icon);
}

static void wintc_sh_drive_monitor_remove_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
)
{
    WinTCShellDrive* sh_drive =
        g_hash_table_lookup(drvmon->map_drive_to_sh_drive, drive);

    if (!sh_drive)
    {
        // Nothing to do
        return;
    }

    // Find and bin the drive icon if needed
    // 
    gchar* obj_path =
        g_drive_get_identifier(
            sh_drive->drive,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category
    );

    g_free(obj_path);
}

static void wintc_sh_drive_monitor_remove_icon(
    WinTCShDriveMonitor* drvmon,
    const gchar*         obj_path,
    const gchar*         guid_category
)
{
    if (
        g_hash_table_remove(
            drvmon->map_id_to_icon,
            obj_path
        )
    )
    {
        wintc_shext_host_remove_toplevel_item(
            drvmon->shext_host,
            guid_category,
            obj_path
        );
    }
}

static void wintc_sh_drive_monitor_remove_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, mount, FALSE);

    if (!sh_drive)
    {
        // FIXME: Need to handle this when network mounts supported
        //
        g_warning("shell: drvmon: mount with no drive removed, unhandled");
        return;
    }

    // Remove the mount icon
    //
    GFile* file     = g_mount_get_default_location(mount);
    gchar* obj_path = g_file_get_path(file);

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category
    );

    g_free(obj_path);
    g_object_unref(file);

    // Add mount icon if there is a volume
    //
    GVolume* volume = g_mount_get_volume(mount);

    if (volume)
    {
        wintc_sh_drive_monitor_add_volume(drvmon, volume);

        g_object_unref(volume);
    }
}

static void wintc_sh_drive_monitor_remove_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
)
{
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, FALSE, volume);

    // Remove the volume icon
    //
    gchar* obj_path =
        g_volume_get_identifier(
            volume,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category
    );

    g_free(obj_path);

    // Add drive icon if there are no more volumes
    //
    if (!g_drive_has_volumes(sh_drive->drive))
    {
        wintc_sh_drive_monitor_register_drive_icon(drvmon, sh_drive);
    }
}

static void wintc_shell_drive_free(
    WinTCShellDrive* sh_drive
)
{
    g_list_free_full(sh_drive->list_unix_paths, (GDestroyNotify) g_free);

    g_free(sh_drive);
}

static void wintc_shell_drive_true_obj_path(
    gchar** obj_path
)
{
    gchar* tmp = g_strdup_printf("file://%s", *obj_path);

    wintc_strsteal(obj_path, &tmp);
}

static void clear_view_item(
    WinTCShextViewItem* item
)
{
    g_free(item->display_name);
    g_free(item->icon_name);
    g_free(item->priv);
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

static void on_volume_monitor_drive_connected(
    WINTC_UNUSED(GVolumeMonitor* self),
    GDrive*  drive,
    gpointer user_data
)
{
    wintc_sh_drive_monitor_add_drive(
        WINTC_SH_DRIVE_MONITOR(user_data),
        drive
    );
}

static void on_volume_monitor_drive_disconnected(
    WINTC_UNUSED(GVolumeMonitor* self),
    GDrive*  drive,
    gpointer user_data
)
{
    WinTCShDriveMonitor* drvmon = WINTC_SH_DRIVE_MONITOR(user_data);

    wintc_sh_drive_monitor_remove_drive(
        drvmon,
        drive
    );

    // Manually destroy the drive tracking since remove_drive only removes
    // the icon representation
    //
    g_hash_table_remove(
        drvmon->map_drive_to_sh_drive,
        drive
    );
}

static void on_volume_monitor_mount_added(
    WINTC_UNUSED(GVolumeMonitor* self),
    GMount*  mount,
    gpointer user_data
)
{
    wintc_sh_drive_monitor_add_mount(
        WINTC_SH_DRIVE_MONITOR(user_data),
        mount
    );
}

static void on_volume_monitor_mount_removed(
    WINTC_UNUSED(GVolumeMonitor* self),
    GMount*  mount,
    gpointer user_data
)
{
    wintc_sh_drive_monitor_remove_mount(
        WINTC_SH_DRIVE_MONITOR(user_data),
        mount
    );
}

static void on_volume_monitor_volume_added(
    WINTC_UNUSED(GVolumeMonitor* self),
    GVolume* volume,
    gpointer user_data
)
{
    wintc_sh_drive_monitor_add_volume(
        WINTC_SH_DRIVE_MONITOR(user_data),
        volume
    );
}

static void on_volume_monitor_volume_removed(
    WINTC_UNUSED(GVolumeMonitor* self),
    GVolume* volume,
    gpointer user_data
)
{
    wintc_sh_drive_monitor_remove_volume(
        WINTC_SH_DRIVE_MONITOR(user_data),
        volume
    );
}
