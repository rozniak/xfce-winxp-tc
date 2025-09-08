#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shellext.h>

#include "../intapi.h"
#include "clock.h"
#include "icon.h"
#include "notifarea.h"
#include "power.h"
#include "volume.h"

#ifndef WINTC_PKGMGR_BSDPKG
#include "network.h"
#endif

//
// FORWARD DECLARATIONS
//
static void wintc_notification_area_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
);

static void wintc_notification_area_dispose(
    GObject* object
);

static GtkWidget* wintc_notification_area_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
);

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCNotificationAreaClass
{
    GtkBinClass __parent__;
};

struct _WinTCNotificationArea
{
    GtkBin __parent__;

    GtkWidget* box_container;
    GtkWidget* label_clock;

    WinTCClockRunner* clock_runner;

    GSList* list_uictl_behaviours;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCNotificationArea,
    wintc_notification_area,
    GTK_TYPE_BIN,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_UI_HOST,
        wintc_notification_area_ishext_ui_host_interface_init
    )
)

static void wintc_notification_area_class_init(
    WinTCNotificationAreaClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_notification_area_dispose;
}

static void wintc_notification_area_init(
    WinTCNotificationArea* self
)
{
    // Set up UI
    //
    self->box_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    self->label_clock   = gtk_label_new(NULL);

    wintc_widget_add_style_class(self->box_container, "wintc-systray");
    wintc_widget_add_style_class(self->label_clock,   "clock");

    gtk_box_pack_end(
        GTK_BOX(self->box_container),
        self->label_clock,
        FALSE,
        FALSE,
        0
    );

    gtk_container_add(
        GTK_CONTAINER(self),
        self->box_container
    );

    // Hook up clock runner
    //
    self->clock_runner =
        wintc_clock_runner_new(GTK_LABEL(self->label_clock));

    // Create notification area icons
    //
#ifndef WINTC_PKGMGR_BSDPKG
    wintc_shext_ui_controller_new_from_type(
        WINTC_TYPE_NOTIFICATION_NETWORK,
        WINTC_ISHEXT_UI_HOST(self)
    );
#endif
    wintc_shext_ui_controller_new_from_type(
        WINTC_TYPE_NOTIFICATION_POWER,
        WINTC_ISHEXT_UI_HOST(self)
    );
    wintc_shext_ui_controller_new_from_type(
        WINTC_TYPE_NOTIFICATION_VOLUME,
        WINTC_ISHEXT_UI_HOST(self)
    );
}

static void wintc_notification_area_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
)
{
    iface->get_ext_widget = wintc_notification_area_get_ext_widget;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_notification_area_dispose(
    GObject* object
)
{
    WinTCNotificationArea* notif_area = WINTC_NOTIFICATION_AREA(object);

    g_clear_object(&(notif_area->clock_runner));
    g_clear_slist(
        &(notif_area->list_uictl_behaviours),
        (GDestroyNotify) g_object_unref
    );

    (G_OBJECT_CLASS(wintc_notification_area_parent_class))->dispose(object);
}

//
// INTERFACE METHODS (WinTCIShextUIHost)
//
static GtkWidget* wintc_notification_area_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
)
{
    WinTCNotificationArea* notif_area = WINTC_NOTIFICATION_AREA(host);

    if (ext_id != WINTC_NOTIFAREA_HOSTEXT_ICON)
    {
        g_critical("notifarea: unsupported ext widget type: %d", ext_id);
        return NULL;
    }

    if (expected_type != WINTC_TYPE_NOTIF_AREA_ICON)
    {
        g_critical("%s", "notifarea: invalid ext widget type");
        return NULL;
    }

    if (!G_IS_OBJECT(ctx))
    {
        g_critical("%s", "notifarea: expected a GObject for icon context");
        return NULL;
    }

    // All good, create the icon
    //
    GtkWidget* notif_icon = wintc_notif_area_icon_new();

    gtk_box_pack_start(
        GTK_BOX(notif_area->box_container),
        notif_icon,
        FALSE,
        FALSE,
        0
    );

    notif_area->list_uictl_behaviours =
        g_slist_append(
            notif_area->list_uictl_behaviours,
            ctx
        );

    return notif_icon;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* notification_area_new(void)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_NOTIFICATION_AREA,
            NULL
        )
    );
}
