#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../toolbar.h"
#include "notifarea.h"
#include "toolbar.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCToolbarNotifAreaClass
{
    WinTCTaskbandToolbarClass __parent__;
};

struct _WinTCToolbarNotifArea
{
    WinTCTaskbandToolbar __parent__;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarNotifArea,
    wintc_toolbar_notif_area,
    WINTC_TYPE_TASKBAND_TOOLBAR
)

static void wintc_toolbar_notif_area_class_init(
    WINTC_UNUSED(WinTCToolbarNotifAreaClass* klass)
) {}

static void wintc_toolbar_notif_area_init(
    WinTCToolbarNotifArea* self
)
{
    WinTCTaskbandToolbar* toolbar = WINTC_TASKBAND_TOOLBAR(self);

    toolbar->widget_root = notification_area_new();
}

