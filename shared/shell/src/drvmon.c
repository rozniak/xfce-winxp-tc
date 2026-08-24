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
    GHashTable* map_id_to_icon;
    GHashTable* map_volume_to_mount_path;
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

static void wintc_sh_drive_monitor_add_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
);
static void wintc_sh_drive_monitor_add_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
);
static void wintc_sh_drive_monitor_add_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
);

static WinTCShellDrive* wintc_sh_drive_get_shell_drive_for_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
);
static WinTCShellDrive* wintc_sh_drive_get_shell_drive_for_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
);

static void wintc_sh_drive_monitor_remove_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
);
static void wintc_sh_drive_monitor_remove_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
);
static void wintc_sh_drive_monitor_remove_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
);

static void wintc_shell_drive_add_icon(
    WinTCShellDrive*           sh_drive,
    const gchar*               obj_path,
    gchar*                     display_name,
    gchar*                     icon_name,
    gchar*                     priv,
    WinTCShextActivateItemFunc activate_cb
);
static void wintc_shell_drive_remove_icon(
    WinTCShellDrive* sh_drive,
    const gchar*     obj_path
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
    WinTCShDriveMonitor* self
)
{
    self->map_drive_to_sh_drive =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            NULL // FIXME: Destroy sh_drive
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

                // We know this is a fixed path
                //
                const gchar* mount_path =
                    g_unix_mount_entry_get_mount_path(mount);

                wintc_shell_drive_add_icon(
                    sh_drive,
                    g_strdup(mount_path),
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

    sh_drive->map_id_to_icon =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            g_free,
            (GDestroyNotify) clear_view_item
        );
    sh_drive->map_volume_to_mount_path =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            g_free
        );

    // Determine drive type and category, insert default icon
    //
    GIcon* icon     = g_drive_get_icon(drive);
    gchar* obj_path = g_drive_get_identifier(
                          drive,
                          G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
                      );
    gchar* text;

    if (g_drive_is_media_removable(drive))
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_MEDIA_CONTAINER;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_REMOVABLES;

        text = g_strdup_printf("Media Drive (%s)", obj_path);

        //
        // FIXME: Activate func needs to be specific to media disks
        //

    }
    else if (g_drive_is_removable(drive))
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

    wintc_shell_drive_add_icon(
        sh_drive,
        obj_path,
        text,
        wintc_icon_get_available_name(icon),
        obj_path,
       (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(icon);

    // Map drive now
    //
    g_hash_table_insert(
        drvmon->map_drive_to_sh_drive,
        drive,
        sh_drive
    );
}

static void wintc_sh_drive_monitor_add_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
    GDrive*  drive;
    gchar*   obj_path;
    GVolume* volume = g_mount_get_volume(mount);

    if (!volume)
    {
        //
        // FIXME: Handle mounts with no volume
        //
        g_warning("%s", "shell: drvmon: mounts w/ no volume unhandled");
        return;
    }

    drive = g_volume_get_drive(volume);

    // Look up owner shell drive
    //
    WinTCShellDrive* sh_drive =
        g_hash_table_lookup(
            drvmon->map_drive_to_sh_drive,
            drive
        );

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

    wintc_shell_drive_remove_icon(
        sh_drive,
        obj_path
    );

    g_free(obj_path);

    // Install mount icon
    //
    GFile* file = g_mount_get_default_location(mount);
    GIcon* icon = g_mount_get_icon(mount);

    obj_path = g_file_get_path(file);

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_shell_drive_add_icon(
        sh_drive,
        obj_path,
        g_mount_get_name(mount),
        wintc_icon_get_available_name(icon),
        obj_path,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(file);
    g_object_unref(icon);

cleanup:
    g_object_unref(drive);
    g_object_unref(volume);
}

static void wintc_sh_drive_monitor_add_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
)
{
   GDrive* drive    = g_volume_get_drive(volume);
   gchar*  obj_path = NULL;

    // Look up owner shell drive
    //
    WinTCShellDrive* sh_drive =
        g_hash_table_lookup(
            drvmon->map_drive_to_sh_drive,
            drive
        );

    if (!sh_drive)
    {
        g_critical("%s", "shell: drvmon: volume with no existing drive!");
        goto cleanup;
    }

    // If the drive has an icon, remove it
    //
    obj_path =
        g_drive_get_identifier(
            sh_drive->drive,
            G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
        );

    wintc_shell_drive_true_obj_path(&obj_path);

    wintc_shell_drive_remove_icon(
        sh_drive,
        obj_path
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

    wintc_shell_drive_add_icon(
        sh_drive,
        obj_path,
        g_volume_get_name(volume),
        wintc_icon_get_available_name(icon),
        obj_path,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_object_unref(icon);

cleanup:
    g_object_unref(drive);
}

static WinTCShellDrive* wintc_sh_drive_get_shell_drive_for_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
}

static WinTCShellDrive* wintc_sh_drive_get_shell_drive_for_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
)
{
}

static void wintc_sh_drive_monitor_remove_drive(
    WinTCShDriveMonitor* drvmon,
    GDrive*              drive
)
{
}

static void wintc_sh_drive_monitor_remove_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
}

static void wintc_sh_drive_monitor_remove_volume(
    WinTCShDriveMonitor* drvmon,
    GVolume*             volume
)
{
}

static void wintc_shell_drive_add_icon(
    WinTCShellDrive*           sh_drive,
    const gchar*               obj_path,
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
        sh_drive->drvmon->shext_host,
        sh_drive->guid_category,
        obj_path,
        item,
        activate_cb,
        NULL // FIXME: Error handling
    );

    g_hash_table_insert(
        sh_drive->map_id_to_icon,
        g_strdup(obj_path),
        item
    );
}

static void wintc_shell_drive_remove_icon(
    WinTCShellDrive* sh_drive,
    const gchar*     obj_path
)
{
    if (
        g_hash_table_remove(
            sh_drive->map_id_to_icon,
            obj_path
        )
    )
    {
        wintc_shext_host_remove_toplevel_item(
            sh_drive->drvmon->shext_host,
            sh_drive->guid_category,
            obj_path
        );
    }
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
    GVolumeMonitor* self,
    GDrive*         drive,
    gpointer        user_data
)
{
}

static void on_volume_monitor_mount_added(
    GVolumeMonitor* self,
    GMount*         mount,
    gpointer        user_data
)
{
}

static void on_volume_monitor_mount_removed(
    GVolumeMonitor* self,
    GMount*         mount,
    gpointer        user_data
)
{
}

static void on_volume_monitor_volume_added(
    GVolumeMonitor* self,
    GVolume*        volume,
    gpointer        user_data
)
{
}

static void on_volume_monitor_volume_removed(
    GVolumeMonitor* self,
    GVolume*        volume,
    gpointer        user_data
)
{
}
