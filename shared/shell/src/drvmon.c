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
    gpointer                   priv,
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

static void clear_view_item(
    WinTCShextViewItem* item
);

static void cb_async_drive_poll(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);
static void cb_async_volume_mount(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
);

static gboolean cb_shext_activate_item_drive(
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gboolean cb_shext_activate_item_mount(
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gboolean cb_shext_activate_item_unix_mount(
    WinTCShextHost*     shext_host,
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    GError**            error
);
static gboolean cb_shext_activate_item_volume(
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
static GHashTable* S_MAP_SHEXT_HOST_TO_DRVMON   = NULL;
static GQuark      S_QUARK_DRVMON_DRIVE_MAPPING = 0;
static GQuark      S_QUARK_DRVMON_VOLUME_UUID   = 0;

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

    // Set up our quark(s)
    //
    S_QUARK_DRVMON_DRIVE_MAPPING =
        g_quark_from_static_string("drvmon-drive-mapping");
    S_QUARK_DRVMON_VOLUME_UUID =
        g_quark_from_static_string("drvmon-volume-uuid");
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
                gchar* mount_path =
                    g_strdup(g_unix_mount_entry_get_mount_path(mount));

                WINTC_LOG_DEBUG(
                    "shell: drvmon: determined unix path %s mapped to %s",
                    g_unix_mount_entry_get_mount_path(mount),
                    g_unix_mount_entry_get_device_path(mount)
                );

                sh_drive->list_unix_paths =
                    g_list_append(
                        sh_drive->list_unix_paths,
                        mount_path
                    );

                // Bin any existing drive icon
                //
                wintc_sh_drive_monitor_remove_drive(drvmon, sh_drive->drive);

                // We know this is a fixed path
                //
                wintc_sh_drive_monitor_add_icon(
                    drvmon,
                    mount_path,
                    sh_drive->guid_category,
                    g_strdup_printf(
                        "Local Disk (%s)",
                        mount_path
                    ),
                    g_strdup("drive-harddisk"),
                    mount_path,
                    (WinTCShextActivateItemFunc)
                        cb_shext_activate_item_unix_mount
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
    // Init global map if necessary
    //
    if (!S_MAP_SHEXT_HOST_TO_DRVMON)
    {
        S_MAP_SHEXT_HOST_TO_DRVMON =
            g_hash_table_new(g_direct_hash, g_direct_equal);
    }

    // Look up existing drive monitor
    //
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

gboolean wintc_sh_drive_monitor_get_path_is_mount(
    WinTCShDriveMonitor* drvmon,
    const gchar*         path
)
{
    GList*          list_mounts    = NULL;
    GList*          list_sh_drives = NULL;
    GVolumeMonitor* monitor        = g_volume_monitor_get();
    gboolean        ret            = FALSE;

    // Check the normal GMounts
    //
    list_mounts = g_volume_monitor_get_mounts(monitor);

    for (GList* iter = list_mounts; iter; iter = iter->next)
    {
        GFile* file = g_mount_get_root((GMount*) iter->data);

        if (g_strcmp0(g_file_peek_path(file), path) == 0)
        {
            ret = TRUE;
        }

        g_object_unref(file);

        if (ret)
        {
            goto cleanup;
        }
    }

    // Didn't find any in the normal mounts - check if it exists in one
    // of the UNIX mount paths we've picked up
    //
    list_sh_drives =
        g_hash_table_get_values(drvmon->map_drive_to_sh_drive);

    for (GList* iter = list_sh_drives; iter; iter = iter->next)
    {
        WinTCShellDrive* sh_drive = (WinTCShellDrive*) iter->data;

        for (
            GList* iter2 = sh_drive->list_unix_paths;
            iter2;
            iter2 = iter2->next
        )
        {
            if (g_strcmp0((gchar*) iter2->data, path) == 0)
            {
                ret = TRUE;
                goto cleanup;
            }
        }
    }

cleanup:
    g_list_free(list_sh_drives);
    g_list_free_full(list_mounts, (GDestroyNotify) g_object_unref);
    g_object_unref(monitor);

    return ret;
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
    gpointer                   priv,
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

    // Look up owner shell drive
    //
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, mount, NULL);

    // Attach references to source drive / mount now - this information gets
    // lost by GIO when the mount is ripped away (such as when Brasero acquires
    // a lock on the CD/DVD drive volume)
    //
    if (sh_drive)
    {
        g_object_set_qdata_full(
            G_OBJECT(mount),
            S_QUARK_DRVMON_DRIVE_MAPPING,
            g_object_ref(sh_drive->drive),
            (GDestroyNotify) g_object_unref
        );
    }

    if (volume)
    {
        g_object_set_qdata_full(
            G_OBJECT(mount),
            S_QUARK_DRVMON_VOLUME_UUID,
            g_volume_get_uuid(volume),
            (GDestroyNotify) g_free
        );
    }

    // If the volume has an icon, remove it
    //
    if (volume)
    {
        obj_path =
            g_volume_get_identifier(
                volume,
                G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
            );

        wintc_sh_drive_monitor_remove_icon(
            drvmon,
            obj_path,
            sh_drive->guid_category
        );

        g_free(obj_path);
    }

    // Install mount icon
    //
    GFile* file = g_mount_get_default_location(mount);
    GIcon* icon = g_mount_get_icon(mount);

    wintc_sh_drive_monitor_add_icon(
        drvmon,
        g_file_peek_path(file),
        sh_drive ? sh_drive->guid_category : WINTC_SH_GUID_CATEGORY_OTHER,
        g_mount_get_name(mount),
        wintc_icon_get_available_name(icon),
        mount,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_mount
    );

    g_object_unref(file);
    g_object_unref(icon);
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
        volume,
        (WinTCShextActivateItemFunc) cb_shext_activate_item_volume
    );

    g_free(obj_path);
    g_object_unref(icon);
}

static WinTCShellDrive* wintc_sh_drive_monitor_get_shell_drive(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount,
    GVolume*             volume
)
{
    GDrive*          drive    = mount ?
                                    g_mount_get_drive(mount) :
                                    g_volume_get_drive(volume);
    WinTCShellDrive* sh_drive = NULL;

    if (drive)
    {
        sh_drive =
            g_hash_table_lookup(drvmon->map_drive_to_sh_drive, drive);

        if (volume)
        {
            g_object_unref(drive);
        }
    }
    else if (mount) // Second attempt
    {
        sh_drive =
            g_hash_table_lookup(
                drvmon->map_drive_to_sh_drive,
                g_object_get_qdata(
                    G_OBJECT(mount),
                    S_QUARK_DRVMON_DRIVE_MAPPING
                )
            );
    }

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
    }
    else if (g_drive_is_removable(sh_drive->drive))
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_REMOVABLE;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_REMOVABLES;

        text = g_strdup_printf("Removable Disk (%s)", obj_path);
    }
    else
    {
        sh_drive->drive_type    = WINTC_SH_DRIVE_TYPE_FIXED;
        sh_drive->guid_category = WINTC_SH_GUID_CATEGORY_DRIVES;

        text = g_strdup_printf("Fixed Disk (%s)", obj_path);
    }

    wintc_sh_drive_monitor_add_icon(
        drvmon,
        obj_path,
        sh_drive->guid_category,
        text,
        wintc_icon_get_available_name(icon),
        sh_drive->drive,
       (WinTCShextActivateItemFunc) cb_shext_activate_item_drive
    );

    g_free(obj_path);
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
    wintc_shext_host_remove_toplevel_item(
        drvmon->shext_host,
        guid_category,
        obj_path
    );

    g_hash_table_remove(
        drvmon->map_id_to_icon,
        obj_path
    );
}

static void wintc_sh_drive_monitor_remove_mount(
    WinTCShDriveMonitor* drvmon,
    GMount*              mount
)
{
    WinTCShellDrive* sh_drive =
        wintc_sh_drive_monitor_get_shell_drive(drvmon, mount, FALSE);

    // Remove the mount icon
    //
    GFile* file     = g_mount_get_default_location(mount);
    gchar* obj_path = g_file_get_path(file);

    wintc_sh_drive_monitor_remove_icon(
        drvmon,
        obj_path,
        sh_drive ? sh_drive->guid_category : WINTC_SH_GUID_CATEGORY_OTHER
    );

    g_free(obj_path);
    g_object_unref(file);

    // Add volume icon if there is a volume
    //
    GVolume* volume = g_mount_get_volume(mount);

    if (volume)
    {
        wintc_sh_drive_monitor_add_volume(drvmon, volume);

        g_object_unref(volume);
    }
    else
    {
        // Either this was a mount with no physical representation (such as
        // a network mount) -- or it was a mount whose information has been
        // lost by GIO
        //
        // Try to claw back the missing information we stored via quarks when
        // this was initially mounted so we can store either the volume or
        // drive icon
        //
        // If neither exist then nothing else needs to be done because it
        // must've been a network mount or something like that
        //
        GVolumeMonitor* monitor     = g_volume_monitor_get();
        const gchar*    volume_uuid = g_object_get_qdata(
                                          G_OBJECT(mount),
                                          S_QUARK_DRVMON_VOLUME_UUID
                                      );

        if (volume_uuid)
        {
            volume =
                g_volume_monitor_get_volume_for_uuid(monitor, volume_uuid);

            if (volume)
            {
                wintc_sh_drive_monitor_add_volume(drvmon, volume);
                g_object_unref(volume);
            }

            goto cleanup;
        }

        if (sh_drive)
        {
            wintc_sh_drive_monitor_register_drive_icon(drvmon, sh_drive);
        }

cleanup:
        g_object_unref(monitor);
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
static void cb_async_drive_poll(
    GObject*      source_object,
    GAsyncResult* res,
    WINTC_UNUSED(gpointer user_data)
)
{
    GError* error = NULL;

    if (!g_drive_poll_for_media_finish((GDrive*) source_object, res, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
    }
}

static void cb_async_volume_mount(
    GObject*      source_object,
    GAsyncResult* res,
    gpointer      user_data
)
{
    GError* error = NULL;

    if (!g_volume_mount_finish((GVolume*) source_object, res, &error))
    {
        wintc_display_error_and_clear(&error, NULL);
    }

    g_object_unref((GObject*) user_data); // Bins the GtkMountOperation
}

static gboolean cb_shext_activate_item_drive(
    WINTC_UNUSED(WinTCShextHost* shext_host),
    WinTCShextViewItem* item,
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    GError**            error
)
{
    GDrive* drive = (GDrive*) item->priv;

    if (g_drive_is_media_removable(drive))
    {
        if (g_drive_is_media_check_automatic(drive))
        {
            // Nothing inserted, user should insert something now!
            //
            // FIXME: Localise string
            // FIXME: Uses a custom dialog in Windows XP, not the msgbox
            //
            gchar* drive_name = g_drive_get_identifier(
                                    drive,
                                    G_DRIVE_IDENTIFIER_KIND_UNIX_DEVICE
                                );
            gchar* msg        = g_strdup_printf(
                                    "Please insert a disk into drive %s.",
                                    drive_name
                                );

            wintc_messagebox_show(
                NULL,
                msg,
                "Insert disk",
                WINTC_BUTTONS_OK,
                WINTC_MESSAGE_NONE
            );

            g_free(msg);
            g_free(drive_name);
        }
        else
        {
            // Probably a floppy drive or something, we should poll now
            //
            g_drive_poll_for_media(
                drive,
                NULL,
                (GAsyncReadyCallback) cb_async_drive_poll,
                NULL
            );
        }

        return TRUE;
    }

    // Else, assume a blank unformatted drive
    //
    // FIXME: Implement this
    //
    g_set_error(
        error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "%s",
        "Sorry, formatting disks is not yet implemented."
    );

    return FALSE;
}

static gboolean cb_shext_activate_item_mount(
    WINTC_UNUSED(WinTCShextHost* shext_host),
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    WINTC_UNUSED(GError** error)
)
{
    GMount* mount = (GMount*) item->priv;
    GFile*  file  = g_mount_get_root(mount);

    path_info->base_path =
        g_strdup_printf("file://%s", g_file_peek_path(file));

    g_object_unref(file);

    return TRUE;
}

static gboolean cb_shext_activate_item_unix_mount(
    WINTC_UNUSED(WinTCShextHost* shext_host),
    WinTCShextViewItem* item,
    WinTCShextPathInfo* path_info,
    WINTC_UNUSED(GError**        error)
)
{
    path_info->base_path =
        g_strdup_printf("file://%s", (gchar*) item->priv);

    return TRUE;
}

static gboolean cb_shext_activate_item_volume(
    WINTC_UNUSED(WinTCShextHost* shext_host),
    WinTCShextViewItem* item,
    WINTC_UNUSED(WinTCShextPathInfo* path_info),
    WINTC_UNUSED(GError** error)
)
{
    GVolume*         volume   = (GVolume*) item->priv;
    GMountOperation* mount_op = gtk_mount_operation_new(NULL);

    g_volume_mount(
        volume,
        G_MOUNT_MOUNT_NONE,
        mount_op,
        NULL,
        (GAsyncReadyCallback) cb_async_volume_mount,
        mount_op // So we can clean up
    );

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
