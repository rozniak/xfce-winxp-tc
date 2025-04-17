#include <gio/gio.h>
#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/fs.h"
#include "../public/monitor.h"
#include "marshal.h"

//
// PRIVATE ENUMS
//
enum
{
    PROP_FILE = 1,
    PROP_FILE_MONITOR
};

enum
{
    SIGNAL_CHANGED = 0,
    N_SIGNALS
};

//
// FORWARD DECLARATIONS
//
static void wintc_sh_dir_monitor_recursive_constructed(
    GObject* object
);
static void wintc_sh_dir_monitor_recursive_dispose(
    GObject* object
);
static void wintc_sh_dir_monitor_recursive_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
);
static void wintc_sh_dir_monitor_recursive_set_property(
    GObject*          object,
    guint             prop_id,
    const GValue*     value,
    GParamSpec*       pspec
);

static void wintc_sh_dir_monitor_recursive_delete_monitor(
    WinTCShDirMonitorRecursive* monitor_recursive,
    GFile*                      file
);
static void wintc_sh_dir_monitor_recursive_new_monitor(
    WinTCShDirMonitorRecursive* monitor_recursive,
    GFile*                      file
);

static void on_file_monitor_root_changed(
    GFileMonitor*     monitor,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);
static void on_file_monitor_subdir_changed(
    GFileMonitor*     monitor,
    GFile*            file,
    GFile*            other_file,
    GFileMonitorEvent event_type,
    gpointer          user_data
);

//
// STATIC DATA
//
static gint wintc_sh_dir_monitor_recursive_signals[N_SIGNALS] = { 0 };

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCShDirMonitorRecursive
{
    GObject __parent__;

    GFile*        file_root;
    GFileMonitor* monitor_root;

    GHashTable* map_rel_path_to_monitor;
};

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCShDirMonitorRecursive,
    wintc_sh_dir_monitor_recursive,
    G_TYPE_OBJECT
)

static void wintc_sh_dir_monitor_recursive_class_init(
    WinTCShDirMonitorRecursiveClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed  = wintc_sh_dir_monitor_recursive_constructed;
    object_class->dispose      = wintc_sh_dir_monitor_recursive_dispose;
    object_class->get_property = wintc_sh_dir_monitor_recursive_get_property;
    object_class->set_property = wintc_sh_dir_monitor_recursive_set_property;

    g_object_class_install_property(
        object_class,
        PROP_FILE,
        g_param_spec_object(
            "file",
            "File",
            "The root directory to monitor recursively.",
            G_TYPE_FILE,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
        )
    );
    g_object_class_install_property(
        object_class,
        PROP_FILE_MONITOR,
        g_param_spec_object(
            "file-monitor",
            "FileMonitor",
            "The file monitor for the root directory.",
            G_TYPE_FILE_MONITOR,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY
        )
    );

    wintc_sh_dir_monitor_recursive_signals[SIGNAL_CHANGED] =
        g_signal_new(
            "changed",
            G_TYPE_FROM_CLASS(object_class),
            G_SIGNAL_RUN_FIRST,
            0,
            NULL,
            NULL,
            wintc_cclosure_marshal_VOID__OBJECT_OBJECT_INT,
            G_TYPE_NONE,
            3,
            G_TYPE_OBJECT,
            G_TYPE_OBJECT,
            G_TYPE_INT
        );
}

static void wintc_sh_dir_monitor_recursive_init(
    WINTC_UNUSED(WinTCShDirMonitorRecursive* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_sh_dir_monitor_recursive_constructed(
    GObject* object
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(object);

    if (
        !(monitor_recursive->file_root) ||
        !(monitor_recursive->monitor_root)
    )
    {
        g_critical("%s", "shcommon: invalid dir monitor created");
        return;
    }

    // Set up signal for the root monitor
    //
    g_signal_connect(
        monitor_recursive->monitor_root,
        "changed",
        G_CALLBACK(on_file_monitor_root_changed),
        monitor_recursive
    );

    // Pull the root path details
    //
    const gchar* root_path = g_file_peek_path(monitor_recursive->file_root);
    gint         root_len  = g_utf8_strlen(root_path, -1);

    // Build monitors recursively in the map
    //
    GList* files =
        wintc_sh_fs_get_names_as_list(
            root_path,
            TRUE,
            G_FILE_TEST_IS_DIR,
            TRUE,
            NULL // FIXME: Error handling
        );

    monitor_recursive->map_rel_path_to_monitor =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            (GDestroyNotify) g_free,
            (GDestroyNotify) g_object_unref
        );

    for (GList* iter = files; iter; iter = iter->next)
    {
        GFile* file =
            g_file_new_for_path(
                (gchar*) iter->data
            );

        GFileMonitor* monitor =
            g_file_monitor_directory(
                file,
                G_FILE_MONITOR_NONE,
                NULL,
                NULL // FIXME: Error handling
            );


        if (monitor)
        {
            g_signal_connect(
                monitor,
                "changed",
                G_CALLBACK(on_file_monitor_subdir_changed),
                monitor_recursive
            );

            g_hash_table_insert(
                monitor_recursive->map_rel_path_to_monitor,
                g_strdup(root_path + root_len),
                monitor
            );
        }

        g_object_unref(file);
    }

    g_list_free_full(files, (GDestroyNotify) g_free);
}

static void wintc_sh_dir_monitor_recursive_dispose(
    GObject* object
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(object);

    g_clear_object(&(monitor_recursive->file_root));
    g_clear_object(&(monitor_recursive->monitor_root));

    g_hash_table_destroy(
        g_steal_pointer(&(monitor_recursive->map_rel_path_to_monitor))
    );

    (G_OBJECT_CLASS(wintc_sh_dir_monitor_recursive_parent_class))
        ->dispose(object);
}

static void wintc_sh_dir_monitor_recursive_get_property(
    GObject*    object,
    guint       prop_id,
    GValue*     value,
    GParamSpec* pspec
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(object);

    switch (prop_id)
    {
        case PROP_FILE:
            g_value_set_object(value, monitor_recursive->file_root);
            break;

        case PROP_FILE_MONITOR:
            g_value_set_object(value, monitor_recursive->monitor_root);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void wintc_sh_dir_monitor_recursive_set_property(
    GObject*          object,
    guint             prop_id,
    const GValue*     value,
    GParamSpec*       pspec
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(object);

    switch (prop_id)
    {
        case PROP_FILE:
            monitor_recursive->file_root = g_value_dup_object(value);
            break;

        case PROP_FILE_MONITOR:
            monitor_recursive->monitor_root = g_value_dup_object(value);
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

//
// PUBLIC FUNCTIONS
//
WinTCShDirMonitorRecursive* wintc_sh_fs_monitor_directory_recursive(
    GFile*            file,
    GFileMonitorFlags flags,
    GCancellable*     cancellable,
    GError**          error
)
{
    GFileMonitor* monitor =
        g_file_monitor_directory(
            file,
            flags,
            cancellable,
            error
        );

    if (!monitor)
    {
        return NULL;
    }

    return WINTC_SH_DIR_MONITOR_RECURSIVE(
        g_object_new(
            WINTC_TYPE_SH_DIR_MONITOR_RECURSIVE,
            "file", file,
            "file-monitor", monitor,
            NULL
        )
    );
}

//
// PRIVATE FUNCTIONS
//
static void wintc_sh_dir_monitor_recursive_delete_monitor(
    WinTCShDirMonitorRecursive* monitor_recursive,
    GFile*                      file
)
{
    const gchar* rel_path =
        g_file_peek_path(file) +
        g_utf8_strlen(
            g_file_peek_path(monitor_recursive->file_root),
            -1
        );

    if (
        !g_hash_table_lookup(
            monitor_recursive->map_rel_path_to_monitor,
            rel_path
        )
    )
    {
        return;
    }

    g_hash_table_remove(
        monitor_recursive->map_rel_path_to_monitor,
        rel_path
    );
}

static void wintc_sh_dir_monitor_recursive_new_monitor(
    WinTCShDirMonitorRecursive* monitor_recursive,
    GFile*                      file
)
{
    const gchar* rel_path =
        g_file_peek_path(file) +
        g_utf8_strlen(
            g_file_peek_path(monitor_recursive->file_root),
            -1
        );

    if (
        g_hash_table_lookup(
            monitor_recursive->map_rel_path_to_monitor,
            rel_path
        )
    )
    {
        return;
    }

    // Attempt to create a directory monitor
    //
    GFileMonitor* monitor =
        g_file_monitor_directory(
            file,
            G_FILE_MONITOR_NONE,
            NULL,
            NULL // FIXME: Error handling
        );

    if (!monitor)
    {
        return;
    }

    g_hash_table_insert(
        monitor_recursive->map_rel_path_to_monitor,
        g_strdup(rel_path),
        monitor
    );
}

//
// CALLBACKS
//
static void on_file_monitor_root_changed(
    WINTC_UNUSED(GFileMonitor* monitor),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(user_data);

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            WINTC_LOG_DEBUG(
                "shcommon: new item in root: %s",
                g_file_peek_path(file)
            );

            wintc_sh_dir_monitor_recursive_new_monitor(
                monitor_recursive,
                file
            );

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            WINTC_LOG_DEBUG(
                "shcommon: deleted item in root: %s",
                g_file_peek_path(file)
            );

            wintc_sh_dir_monitor_recursive_delete_monitor(
                monitor_recursive,
                file
            );

            break;

        default:
            WINTC_LOG_DEBUG(
                "shcommon: unhandled event in root: %s (%d)",
                g_file_peek_path(file),
                event_type
            );
            break;
    }

    g_signal_emit(
        monitor_recursive,
        wintc_sh_dir_monitor_recursive_signals[SIGNAL_CHANGED],
        0,
        file,
        other_file,
        event_type
    );
}

static void on_file_monitor_subdir_changed(
    WINTC_UNUSED(GFileMonitor* monitor),
    GFile*            file,
    WINTC_UNUSED(GFile* other_file),
    GFileMonitorEvent event_type,
    gpointer          user_data
)
{
    WinTCShDirMonitorRecursive* monitor_recursive =
        WINTC_SH_DIR_MONITOR_RECURSIVE(user_data);

    switch (event_type)
    {
        case G_FILE_MONITOR_EVENT_CREATED:
            WINTC_LOG_DEBUG(
                "shcommon: new item in subdir: %s",
                g_file_peek_path(file)
            );

            wintc_sh_dir_monitor_recursive_new_monitor(
                monitor_recursive,
                file
            );

            break;

        case G_FILE_MONITOR_EVENT_DELETED:
            WINTC_LOG_DEBUG(
                "shcommon: deleted item in subdir: %s",
                g_file_peek_path(file)
            );

            wintc_sh_dir_monitor_recursive_delete_monitor(
                monitor_recursive,
                file
            );

            break;

        default:
            WINTC_LOG_DEBUG(
                "shcommon: unhandled event in subdir: %s (%d)",
                g_file_peek_path(file),
                event_type
            );
            break;
    }

    g_signal_emit(
        monitor_recursive,
        wintc_sh_dir_monitor_recursive_signals[SIGNAL_CHANGED],
        0,
        file,
        other_file,
        event_type
    );
}
