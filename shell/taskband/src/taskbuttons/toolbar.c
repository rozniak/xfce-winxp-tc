#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "taskbuttonbar.h"
#include "toolbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_task_buttons_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarTaskButtons
{
    WinTCShextUIController __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarTaskButtons,
    wintc_toolbar_task_buttons,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_task_buttons_class_init(
    WinTCToolbarTaskButtonsClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_task_buttons_constructed;
}

static void wintc_toolbar_task_buttons_init(
    WINTC_UNUSED(WinTCToolbarTaskButtons* self)
)
{
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
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_task_buttons_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_task_buttons_parent_class))
        ->constructed(object);

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        taskbutton_bar_new()
    );
}
