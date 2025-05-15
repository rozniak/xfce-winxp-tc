#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>

#include "procmon.h"

//
// FORWARD DECLARATIONS
//
static void process_get_details(
    guint   pid,
    gchar** owner,
    guint*  cpu_time,
    guint*  mem_total
);

static void process_get_totals(
    guint* cpu_time
);

static gboolean timeout_process_monitor(
    gpointer user_data
);

//
// STATIC DATA
//
static gchar* S_PROC_STAT_PATH = NULL;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCTaskmgrProcmon
{
    GObject __parent__;

    // State
    //
    guint         id_procmon;
    guint         last_cpu_time;
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
)
{
    S_PROC_STAT_PATH =
        g_build_path(
            G_DIR_SEPARATOR_S,
            G_DIR_SEPARATOR_S,
            "proc",
            "stat",
            NULL
        );
}

static void wintc_taskmgr_procmon_init(
    WinTCTaskmgrProcmon* self
)
{
    // Set up model
    //
    self->model_procs =
        gtk_list_store_new(
            N_COLUMNS,
            G_TYPE_UINT,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_UINT,
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
// PRIVATE FUNCTIONS
//
static void process_get_details(
    guint   pid,
    gchar** owner,
    guint*  cpu_time,
    guint*  mem_total
)
{
    GError* error   = NULL;
    gchar*  pid_str = g_strdup_printf("%u", pid);

    // Owner
    //
    if (owner)
    {
        GFile*     file_pid  = g_file_new_build_filename(
                                   G_DIR_SEPARATOR_S,
                                   "proc",
                                   pid_str,
                                   NULL
                               );
        GFileInfo* file_info = g_file_query_info(
                                   file_pid,
                                   G_FILE_ATTRIBUTE_OWNER_USER,
                                   G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS,
                                   NULL,
                                   &error
                               );


        if (file_info)
        {
            *owner =
                g_file_info_get_attribute_as_string(
                    file_info,
                    G_FILE_ATTRIBUTE_OWNER_USER
                );

            g_object_unref(file_info);
        }
        else
        {
#ifdef WINTC_CHECKED
            g_warning("Problem querying file info for process %s", pid_str);
            wintc_log_error_and_clear(&error);
#else
            g_clear_error(&error);
#endif
        }

        g_object_unref(file_pid);
    }

    // Stats
    //
    // FIXME: This is pretty rough, we use utime + stime / total CPU time for
    //        usage, and we're just displaying virtual memory usage in KB for
    //        memory total
    //
    //        This is likely pretty far from how Windows displays these stats,
    //        but it's better than nothing for now
    //
    gchar* stat_path = g_build_path(
                           G_DIR_SEPARATOR_S,
                           G_DIR_SEPARATOR_S,
                           "proc",
                           pid_str,
                           "stat",
                           NULL
                       );
    gchar* stat_str  = NULL;

    *cpu_time  = 0;
    *mem_total = 0;

    if (g_file_get_contents(stat_path, &stat_str, NULL, NULL))
    {
        gchar* mem_s   = wintc_strdup_delimited(stat_str, " ", 22);
        gchar* utime_s = wintc_strdup_delimited(stat_str, " ", 13);
        gchar* stime_s = wintc_strdup_delimited(stat_str, " ", 14);

        *cpu_time  = strtoul(utime_s, NULL, 0) + strtoul(stime_s, NULL, 0);
        *mem_total = strtoul(mem_s, NULL, 0) / 1024;

        g_free(stat_str);
        g_free(mem_s);
        g_free(utime_s);
        g_free(stime_s);
    }

    g_free(stat_path);
    g_free(pid_str);
}

static void process_get_totals(
    guint* cpu_time
)
{
    gchar* cpu_stats = NULL;

    if (
        g_file_get_contents(
            S_PROC_STAT_PATH,
            &cpu_stats,
            NULL,
            NULL
        )
    )
    {
        gchar* last_stime_s = wintc_strdup_delimited(
                                  cpu_stats + 5, // skip 'cpu  '
                                  " ",
                                  0
                              );
        gchar* last_utime_s = wintc_strdup_delimited(
                                  cpu_stats + 5,
                                  " ",
                                  2
                              );

        *cpu_time =
            strtoul(last_stime_s, NULL, 0) + strtoul(last_utime_s, NULL, 0);

        g_free(last_stime_s);
        g_free(last_utime_s);
        g_free(cpu_stats);
    }
}

//
// CALLBACKS
//
static gboolean timeout_process_monitor(
    gpointer user_data
)
{
    WinTCTaskmgrProcmon* procmon = WINTC_TASKMGR_PROCMON(user_data);

    // Acquire the total CPU time stats
    //
    guint total_cpu_delta = 0;
    guint total_cpu_time  = 0;

    process_get_totals(
        &total_cpu_time
    );

    if (total_cpu_time)
    {
        total_cpu_delta = total_cpu_time - procmon->last_cpu_time;
    }

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

                continue;
            }

            g_free(exe);

            // We DO have a mapping AND it matches, so update the row (and
            // remove the mapping so we know it has been dealt with)
            //
            guint  cpu_time;
            guint  last_cpu_time;
            guint  mem_usage;

            process_get_details(
                pid,
                NULL,
                &cpu_time,
                &mem_usage
            );

            gtk_tree_model_get(
                GTK_TREE_MODEL(procmon->model_procs),
                &tree_iter,
                COLUMN_CPU_TIME, &last_cpu_time,
                -1
            );

            // Update the records
            //
            guint   cpu_delta = cpu_time - last_cpu_time;
            gdouble cpu_pct   = cpu_delta / (gdouble) total_cpu_delta;

            gtk_list_store_set(
                procmon->model_procs,
                &tree_iter,
                COLUMN_CPU_TIME,  cpu_time,
                COLUMN_CPU_USAGE, (gint) (cpu_pct * 100),
                COLUMN_MEM_USAGE, mem_usage,
                -1
            );

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

        guint  cpu_time;
        guint  mem_total;
        gchar* owner = NULL;

        process_get_details(
            pid,
            &owner,
            &cpu_time,
            &mem_total
        );

        gtk_list_store_append(
            procmon->model_procs,
            &tree_iter
        );
        gtk_list_store_set(
            procmon->model_procs,
            &tree_iter,
            COLUMN_PID,        pid,
            COLUMN_IMAGE_NAME, exe,
            COLUMN_USER_NAME,  owner,
            COLUMN_CPU_TIME,   cpu_time,
            COLUMN_CPU_USAGE,  0,
            COLUMN_MEM_USAGE,  mem_total,
            -1
        );

        g_free(owner);
    }

    g_hash_table_destroy(map_pid_to_exe);

    // Update processor stats
    //
    procmon->last_cpu_time = total_cpu_time;

    return G_SOURCE_CONTINUE;
}
