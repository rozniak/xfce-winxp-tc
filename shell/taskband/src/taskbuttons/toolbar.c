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

    // Apply stylesheet for this toolbar
    //
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/taskband/task-buttons.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    // Create root widget
    //
    toolbar->widget_root = taskbutton_bar_new();
}

