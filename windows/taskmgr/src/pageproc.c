#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "intapi.h"
#include "pageproc.h"

//
// PRIVATE ENUMS
//
enum
{
    COLUMN_IMAGE_NAME = 0,
    COLUMN_USER_NAME,
    COLUMN_CPU_USAGE,
    COLUMN_MEM_USAGE,
    N_COLUMNS
};

//
// FORWARD DECLARATIONS
//
static void wintc_taskmgr_page_processes_constructed(
    GObject* object
);
static void wintc_taskmgr_page_processes_dispose(
    GObject* object
);

static gboolean timeout_process_monitor(
    gpointer user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCTaskmgrPageProcesses
{
    WinTCShextUIController __parent__;

    // Process monitoring
    //
    guint         id_procmon;
    GtkListStore* model_procs;

    // UI
    //
    GtkWidget* treeview_procs;
} WinTCTaskmgrPageProcess;

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_TYPE(
     WinTCTaskmgrPageProcesses,
     wintc_taskmgr_page_processes,
     WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_taskmgr_page_processes_class_init(
    WinTCTaskmgrPageProcessesClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_taskmgr_page_processes_constructed;
    object_class->dispose     = wintc_taskmgr_page_processes_dispose;
}

static void wintc_taskmgr_page_processes_init(
    WINTC_UNUSED(WinTCTaskmgrPageProcesses* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_taskmgr_page_processes_constructed(
    GObject* object
)
{
    WinTCTaskmgrPageProcesses* page = WINTC_TASKMGR_PAGE_PROCESSES(object);

    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskmgr/page-processes.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    // Insert page into host
    //
    WinTCIShextUIHost* ui_host =
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        );

    wintc_builder_get_objects(
        builder,
        "tree-view", &(page->treeview_procs),
        NULL
    );

    wintc_ishext_ui_host_get_ext_widget(
        ui_host,
        WINTC_TASKMGR_HOSTEXT_PAGE,
        GTK_TYPE_BOX,
        builder
    );

    g_object_unref(builder);

    // Set up treeview
    //
    page->model_procs =
        gtk_list_store_new(
            4,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_INT,
            G_TYPE_INT
        );

    gtk_tree_view_set_model(
        GTK_TREE_VIEW(page->treeview_procs),
        GTK_TREE_MODEL(page->model_procs)
    );

    gtk_tree_view_append_column(
        GTK_TREE_VIEW(page->treeview_procs),
        gtk_tree_view_column_new_with_attributes(
            "Image Name",
            gtk_cell_renderer_text_new(),
            "text", COLUMN_IMAGE_NAME,
            NULL
        )
    );
    gtk_tree_view_append_column(
        GTK_TREE_VIEW(page->treeview_procs),
        gtk_tree_view_column_new_with_attributes(
            "User Name",
            gtk_cell_renderer_text_new(),
            "text", COLUMN_USER_NAME,
            NULL
        )
    );
    gtk_tree_view_append_column(
        GTK_TREE_VIEW(page->treeview_procs),
        gtk_tree_view_column_new_with_attributes(
            "CPU",
            gtk_cell_renderer_text_new(),
            "text", COLUMN_CPU_USAGE,
            NULL
        )
    );
    gtk_tree_view_append_column(
        GTK_TREE_VIEW(page->treeview_procs),
        gtk_tree_view_column_new_with_attributes(
            "Mem Usage",
            gtk_cell_renderer_text_new(),
            "text", COLUMN_MEM_USAGE,
            NULL
        )
    );

    // Start process monitor
    //
    page->id_procmon =
        g_timeout_add_seconds(
            1,
            (GSourceFunc) timeout_process_monitor,
            page
        );

    // Chain up
    //
    (G_OBJECT_CLASS(wintc_taskmgr_page_processes_parent_class))
        ->constructed(object);
}

static void wintc_taskmgr_page_processes_dispose(
    GObject* object
)
{
    WinTCTaskmgrPageProcesses* page = WINTC_TASKMGR_PAGE_PROCESSES(object);

    if (page->id_procmon)
    {
        g_source_remove(page->id_procmon);
        page->id_procmon = 0;
    }

    (G_OBJECT_CLASS(wintc_taskmgr_page_processes_parent_class))
        ->dispose(object);
}

//
// CALLBACKS
//
static gboolean timeout_process_monitor(
    gpointer user_data
)
{
    WinTCTaskmgrPageProcesses* page = WINTC_TASKMGR_PAGE_PROCESSES(user_data);

    // FIXME: In future, only update the model, rather than a total refresh
    //
    gtk_list_store_clear(page->model_procs);

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
                                   "standard::symlink-target,owner::user",
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
        const gchar* exe_owner  = g_file_info_get_attribute_string(
                                      file_info,
                                      "owner::user"
                                  );
        gchar*       exe_target = g_file_info_get_attribute_as_string(
                                      file_info,
                                      "standard::symlink-target"
                                  );

        // Insert into model
        //
        if (exe_owner && exe_target)
        {
            GtkTreeIter tree_iter;

            gtk_list_store_append(
                page->model_procs,
                &tree_iter
            );
            gtk_list_store_set(
                page->model_procs,
                &tree_iter,
                COLUMN_IMAGE_NAME, wintc_basename(exe_target),
                COLUMN_USER_NAME,  exe_owner,
                COLUMN_CPU_USAGE,  0,
                COLUMN_MEM_USAGE,  0,
                -1
            );
        }

        g_free(exe_target);
        g_object_unref(file_info);
    }

    g_list_free_full(proc_dirs, (GDestroyNotify) g_free);

    return G_SOURCE_CONTINUE;
}
