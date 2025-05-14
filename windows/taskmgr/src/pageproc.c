#include <glib.h>
#include <gtk/gtk.h>
#include <signal.h>
#include <wintc/comgtk.h>
#include <wintc/shcommon.h>
#include <wintc/shell.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "intapi.h"
#include "pageproc.h"
#include "procmon.h"

//
// FORWARD DECLARATIONS
//
static void wintc_taskmgr_page_processes_constructed(
    GObject* object
);

static void on_button_endproc_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_dlgkill_button_yes_clicked(
    GtkButton* self,
    gpointer   user_data
);
static void on_treeview_cursor_changed(
    GtkWidget* self,
    gpointer   user_data
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCTaskmgrPageProcesses
{
    WinTCShextUIController __parent__;

    // UI
    //
    GtkWidget* button_endproc;
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
        "button-endproc", &(page->button_endproc),
        "tree-view",      &(page->treeview_procs),
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
    WinTCTaskmgrProcmon* procmon = wintc_taskmgr_procmon_get_instance();

    wintc_taskmgr_procmon_bind_tree_view_model(
        procmon,
        GTK_TREE_VIEW(page->treeview_procs)
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

    // Hook up signals
    //
    g_signal_connect(
        page->button_endproc,
        "clicked",
        G_CALLBACK(on_button_endproc_clicked),
        page
    );

    g_signal_connect(
        page->treeview_procs,
        "cursor-changed",
        G_CALLBACK(on_treeview_cursor_changed),
        page
    );

    // Chain up
    //
    (G_OBJECT_CLASS(wintc_taskmgr_page_processes_parent_class))
        ->constructed(object);
}

//
// CALLBACKS
//
static void on_button_endproc_clicked(
    GtkButton* self,
    gpointer   user_data
)
{
    WinTCTaskmgrPageProcesses* page = WINTC_TASKMGR_PAGE_PROCESSES(user_data);

    GtkTreeIter   iter;
    GtkTreeModel* model = NULL;

    if (
        !gtk_tree_selection_get_selected(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(page->treeview_procs)),
            &model,
            &iter
        )
    )
    {
        return;
    }

    // Grab the PID and kill
    //
    guint pid = 0;

    gtk_tree_model_get(
        model,
        &iter,
        COLUMN_PID, &pid,
        -1
    );

    // Spawn the dialog to confirm the action
    //
    GtkBuilder* builder;

    GtkWidget* button_no  = NULL;
    GtkWidget* button_yes = NULL;
    GtkWidget* wnd        = NULL;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskmgr/dlgkill.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "main-wnd",   &wnd,
        "button-no",  &button_no,
        "button-yes", &button_yes,
        NULL
    );

    gtk_window_set_modal(
        GTK_WINDOW(wnd),
        TRUE
    );
    gtk_window_set_transient_for(
        GTK_WINDOW(wnd),
        wintc_widget_get_toplevel_window(GTK_WIDGET(self))
    );

    g_signal_connect(
        button_no,
        "clicked",
        G_CALLBACK(wintc_button_close_window_on_clicked),
        NULL
    );
    g_signal_connect(
        button_yes,
        "clicked",
        G_CALLBACK(on_dlgkill_button_yes_clicked),
        GUINT_TO_POINTER(pid)
    );

    gtk_window_present_with_time(
        GTK_WINDOW(wnd),
        0
    );

    wintc_sh_play_sound(WINTC_SHELL_SND_EXCLAM);

    g_object_unref(builder);
}

static void on_dlgkill_button_yes_clicked(
    GtkButton* self,
    gpointer   user_data
)
{
    guint pid = GPOINTER_TO_UINT(user_data);

    gtk_window_close(
        wintc_widget_get_toplevel_window(GTK_WIDGET(self))
    );

    kill(pid, SIGKILL);
}

static void on_treeview_cursor_changed(
    GtkWidget* self,
    gpointer   user_data
)
{
    WinTCTaskmgrPageProcesses* page = WINTC_TASKMGR_PAGE_PROCESSES(user_data);

    gtk_widget_set_sensitive(
        page->button_endproc,
        gtk_tree_selection_get_selected(
            gtk_tree_view_get_selection(GTK_TREE_VIEW(self)),
            NULL,
            NULL
        )
    );
}
