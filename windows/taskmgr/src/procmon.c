#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>

#include "procmon.h"

//
// FORWARD DECLARATIONS
//
static gboolean timeout_process_monitor(
    gpointer user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCTaskmgrProcmon
{
    GObject __parent__;

    // State
    //
    guint         id_procmon;
    GtkListStore* model_procs;
} WinTCTaskmgrProcmon;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
    WinTCTaskmgrProcmon,
    wintc_taskmgr_procmon,
    G_TYPE_OBJECT
)

static void wintc_taskmgr_procmon_class_init(
    WINTC_UNUSED(WinTCTaskmgrProcmonClass* klass)
) {}

static void wintc_taskmgr_procmon_init(
    WinTCTaskmgrProcmon* self
)
{
    // Set up model
    //
    self->model_procs =
        gtk_list_store_new(
            5,
            G_TYPE_UINT,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_INT,
            G_TYPE_INT
        );

    // Start process monitor
    //
    self->id_procmon =
        g_timeout_add_seconds(
            1,
            (GSourceFunc) timeout_process_monitor,
            self
        );
}

//
// PUBLIC FUNCTIONS
//
WinTCTaskmgrProcmon* wintc_taskmgr_procmon_get_instance(void)
{
    static WinTCTaskmgrProcmon* singleton;

    if (!singleton)
    {
        singleton =
            WINTC_TASKMGR_PROCMON(
                g_object_new(
                    WINTC_TYPE_TASKMGR_PROCMON,
                    NULL
                )
            );
    }

    return singleton;
}

void wintc_taskmgr_procmon_bind_tree_view_model(
    WinTCTaskmgrProcmon* procmon,
    GtkTreeView*         tree_view
)
{
    gtk_tree_view_set_model(
        tree_view,
        GTK_TREE_MODEL(procmon->model_procs)
    );
}

//
// CALLBACKS
//
static gboolean timeout_process_monitor(
    gpointer user_data
)
{
    WinTCTaskmgrProcmon* procmon = WINTC_TASKMGR_PROCMON(user_data);

    // We store a mapping of PIDs that we found to check against later
    //
    GHashTable* map_pid_to_exe =
        g_hash_table_new_full(
            g_direct_hash,
            g_direct_equal,
            NULL,
            (GDestroyNotify) g_free
        );

    // Retrieve stuff under /proc
    //
    GList* proc_dirs =
        wintc_sh_fs_get_names_as_list(
            G_DIR_SEPARATOR_S "proc",
            TRUE,
            G_FILE_TEST_IS_DIR,
            FALSE,
            NULL
        );

    for (GList* iter = proc_dirs; iter; iter = iter->next)
    {
        const gchar* dir_name = wintc_basename((gchar*) iter->data);

        // Only interested in PIDs
        //
        if (!g_ascii_isdigit(*dir_name))
        {
            continue;
        }

        // Retrieve the exe info
        //
        GFile*     file_exe  = g_file_new_build_filename(
                                   G_DIR_SEPARATOR_S,
                                   (gchar*) iter->data,
                                   "exe",
                                   NULL
                               );
        GFileInfo* file_info = g_file_query_info(
                                   file_exe,
                                   "standard::symlink-target",
                                   G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                   NULL,
                                   NULL
                               );

        g_object_unref(file_exe);

        if (!file_info)
        {
            continue;
        }

        // Pull the info
        //
        gchar* exe_target =
            g_file_info_get_attribute_as_string(
                file_info,
                "standard::symlink-target"
            );

        // Track this process
        //
        if (exe_target)
        {
            g_hash_table_insert(
                map_pid_to_exe,
                GUINT_TO_POINTER(
                    strtoul(dir_name, NULL, 0)
                ),
                g_strdup(wintc_basename(exe_target))
            );

            g_free(exe_target);
        }

        g_object_unref(file_info);
    }

    g_list_free_full(proc_dirs, (GDestroyNotify) g_free);

    // Iterate over the current list store, update rows where we have a
    // mapping, remove ones we don't, and then insert the remainder
    //
    GtkTreeIter tree_iter;

    if (
        gtk_tree_model_get_iter_first(
            GTK_TREE_MODEL(procmon->model_procs),
            &tree_iter
        )
    )
    {
        while (TRUE)
        {
            guint  pid = 0;
            gchar* exe = NULL;

            gtk_tree_model_get(
                GTK_TREE_MODEL(procmon->model_procs),
                &tree_iter,
                COLUMN_PID,        &pid,
                COLUMN_IMAGE_NAME, &exe,
                -1
            );

            // If we either don't have a mapping or it doesn't match, delete
            // the row
            //
            const gchar* cmp_exe =
                g_hash_table_lookup(map_pid_to_exe, GUINT_TO_POINTER(pid));

            if (
                !cmp_exe ||
                g_strcmp0(exe, cmp_exe) != 0
            )
            {
                if (
                    !gtk_list_store_remove(
                        procmon->model_procs,
                        &tree_iter
                    )
                )
                {
                    break;
                }
            }

            // We DO have a mapping AND it matches, so update the row (and
            // remove the mapping so we know it has been dealt with)
            //

            // FIXME: Update CPU/RAM usage stats here

            g_hash_table_remove(
                map_pid_to_exe,
                GUINT_TO_POINTER(pid)
            );

            // Next...
            //
            if (
                !gtk_tree_model_iter_next(
                    GTK_TREE_MODEL(procmon->model_procs),
                    &tree_iter
                )
            )
            {
                break;
            }
        }
    }

    // Iterate over the remaining items in the map to add them
    //
    GHashTableIter map_iter;
    gpointer       map_key_pid;
    gpointer       map_value_exe;

    g_hash_table_iter_init(&map_iter, map_pid_to_exe);

    while (
        g_hash_table_iter_next(
            &map_iter,
            &map_key_pid,
            &map_value_exe
        )
    )
    {
        const gchar* exe = (gchar*) map_value_exe;
        guint        pid = GPOINTER_TO_UINT(map_key_pid);

        gtk_list_store_append(
            procmon->model_procs,
            &tree_iter
        );
        gtk_list_store_set(
            procmon->model_procs,
            &tree_iter,
            COLUMN_PID,        pid,
            COLUMN_IMAGE_NAME, exe,
            COLUMN_USER_NAME,  "FIXME!",
            COLUMN_CPU_USAGE,  0,
            COLUMN_MEM_USAGE,  0,
            -1
        );
    }

    g_hash_table_destroy(map_pid_to_exe);

    return G_SOURCE_CONTINUE;
}
