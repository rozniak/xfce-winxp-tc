#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../toolbar.h"
#include "taskbuttonbar.h"
#include "toolbar.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarTaskButtonsClass
{
    WinTCTaskbandToolbarClass __parent__;
};

struct _WinTCToolbarTaskButtons
{
    WinTCTaskbandToolbar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarTaskButtons,
    wintc_toolbar_task_buttons,
    WINTC_TYPE_TASKBAND_TOOLBAR
)

static void wintc_toolbar_task_buttons_class_init(
    WINTC_UNUSED(WinTCToolbarTaskButtonsClass* klass)
) {}

static void wintc_toolbar_task_buttons_init(
    WinTCToolbarTaskButtons* self
)
{
    WinTCTaskbandToolbar* toolbar = WINTC_TASKBAND_TOOLBAR(self);

    // Create root widget
    //
    toolbar->widget_root = taskbutton_bar_new();
}

