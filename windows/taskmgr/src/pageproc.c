#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "intapi.h"
#include "pageproc.h"

//
// FORWARD DECLARATIONS
//
static void wintc_taskmgr_page_processes_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
typedef struct _WinTCTaskmgrPageProcesses
{
    WinTCShextUIController __parent__;
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

    wintc_ishext_ui_host_get_ext_widget(
        ui_host,
        WINTC_TASKMGR_HOSTEXT_PAGE,
        GTK_TYPE_BOX,
        builder
    );

    g_object_unref(builder);

    // Chain up
    //
    (G_OBJECT_CLASS(wintc_taskmgr_page_processes_parent_class))
        ->constructed(object);
}
