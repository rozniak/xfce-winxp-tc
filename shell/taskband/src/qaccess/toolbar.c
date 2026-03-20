#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "toolbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_quick_access_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarQuickAccess
{
    WinTCShextUIController __parent__;

    // UI stuff
    //
    GtkWidget* box_programs;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarQuickAccess,
    wintc_toolbar_quick_access,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_quick_access_class_init(
    WinTCToolbarQuickAccessClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_quick_access_constructed;
}

static void wintc_toolbar_quick_access_init(
    WINTC_UNUSED(WinTCToolbarQuickAccess* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_quick_access_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_quick_access_parent_class))
        ->constructed(object);

    WinTCToolbarQuickAccess* toolbar_qaccess =
        WINTC_TOOLBAR_QUICK_ACCESS(object);

    toolbar_qaccess->box_programs = 
        gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        toolbar_qaccess->box_programs
    );

    // Test button
    //
    GtkWidget* button = 
        gtk_button_new_from_icon_name("iexplore", GTK_ICON_SIZE_MENU);

    gtk_container_add(
        GTK_CONTAINER(toolbar_qaccess->box_programs),
        button
    );

    gtk_widget_show(button);
}
