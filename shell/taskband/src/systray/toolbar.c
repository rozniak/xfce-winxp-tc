#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "notifarea.h"
#include "toolbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_notif_area_constructed(
    GObject* object
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarNotifArea
{
    WinTCShextUIController __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarNotifArea,
    wintc_toolbar_notif_area,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_notif_area_class_init(
    WinTCToolbarNotifAreaClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_notif_area_constructed;
}

static void wintc_toolbar_notif_area_init(
    WINTC_UNUSED(WinTCToolbarNotifArea* self)
) {}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_notif_area_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_notif_area_parent_class))
        ->constructed(object);

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        notification_area_new()
    );
}
