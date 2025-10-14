#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>

#include "application.h"
#include "intapi.h"
#include "window.h"
#include "start/toolbar.h"
#include "systray/toolbar.h"
#include "taskbuttons/toolbar.h"

#define TASKBAND_HEIGHT 30

//
// PRIVATE ENUMS
//
typedef enum
{
    WINTC_TASKBAND_TOOLBAR_START,
    WINTC_TASKBAND_TOOLBAR_QUICK_ACCESS,
    WINTC_TASKBAND_TOOLBAR_PRE_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_POST_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_NOTIFICATION_AREA
} WinTCTaskbandToolbarId;

//
// FORWARD DECLARATIONS
//
static void wintc_taskband_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
);

static void wintc_taskband_window_dispose(
    GObject* object
);

static GtkWidget* wintc_taskband_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
);

static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);

//
// STATIC DATA
//
static const WinTCTaskbandToolbarId S_LAYOUT[] = {
    WINTC_TASKBAND_TOOLBAR_START,
    WINTC_TASKBAND_TOOLBAR_QUICK_ACCESS,
    WINTC_TASKBAND_TOOLBAR_PRE_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_POST_BUTTONS,
    WINTC_TASKBAND_TOOLBAR_NOTIFICATION_AREA
};

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskbandWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCTaskbandWindow
{
    GtkApplicationWindow __parent__;

    // UI
    //
    GtkWidget* main_box;

    GSList* toolbars;

    WinTCShextUIController* uictl_notifarea;
    WinTCShextUIController* uictl_start;
    WinTCShextUIController* uictl_taskbuttons;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCTaskbandWindow,
    wintc_taskband_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_UI_HOST,
        wintc_taskband_window_ishext_ui_host_interface_init
    )
)

static void wintc_taskband_window_class_init(
    WinTCTaskbandWindowClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = wintc_taskband_window_dispose;
}

static void wintc_taskband_window_init(
    WinTCTaskbandWindow* self
)
{
    //
    // WINDOW SETUP
    //
    wintc_widget_add_style_class(
        GTK_WIDGET(self),
        "wintc-taskband"
    );

    // FIXME: This is obviously hard coded rubbish!
    //
    gtk_widget_set_size_request(GTK_WIDGET(self), -1, TASKBAND_HEIGHT);
    wintc_anchor_taskband_to_bottom(GTK_WINDOW(self));

    // Create main container box
    //
    self->main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_add(
        GTK_CONTAINER(self),
        self->main_box
    );

    // Connect to map-event signal to spawn toolbars once the window opens
    //
    g_signal_connect(
        self,
        "map-event",
        G_CALLBACK(on_window_map_event),
        NULL
    );
}

static void wintc_taskband_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
)
{
    iface->get_ext_widget = wintc_taskband_window_get_ext_widget;
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_taskband_window_dispose(
    GObject* object
)
{
    WinTCTaskbandWindow* taskband = WINTC_TASKBAND_WINDOW(object);

    g_clear_slist(&(taskband->toolbars), NULL);

    g_clear_object(&(taskband->uictl_notifarea));
    g_clear_object(&(taskband->uictl_start));
    g_clear_object(&(taskband->uictl_taskbuttons));

    (G_OBJECT_CLASS(wintc_taskband_window_parent_class))->dispose(object);
}

//
// INTERFACE METHODS (WinTCIShextUIHost)
//
static GtkWidget* wintc_taskband_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
)
{
    WinTCTaskbandWindow* taskband = WINTC_TASKBAND_WINDOW(host);

    // Only toolbars are supported
    //
    if (ext_id != WINTC_TASKBAND_HOSTEXT_TOOLBAR)
    {
        g_critical("taskband: unsupported ext widget type: %d", ext_id);
        return NULL;
    }

    // No special type expected, we host any kind of widget as a 'toolbar'
    //
    if (expected_type != GTK_TYPE_WIDGET || !GTK_IS_WIDGET(ctx))
    {
        g_critical("%s", "taskband: invalid ext widget type");
        return NULL;
    }

    // Go ahead and import the widget
    //
    // FIXME: We expand based on hexpand... this isn't SUPER intuitive when
    //        vertical taskbar support is implemented, but it'll do
    //
    GtkWidget* toolbar = GTK_WIDGET(ctx);
    gboolean   expand  = gtk_widget_get_hexpand(toolbar);

    taskband->toolbars =
        g_slist_append(
            taskband->toolbars,
            toolbar
        );

    gtk_box_pack_start(
        GTK_BOX(taskband->main_box),
        toolbar,
        expand,
        expand,
        0
    );

    gtk_widget_show_all(toolbar);

    return ctx;
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskband_window_new(
    WinTCTaskbandApplication* app
)
{
    return GTK_WIDGET(
        g_object_new(
            WINTC_TYPE_TASKBAND_WINDOW,
            "application", GTK_APPLICATION(app),
            "type",        GTK_WINDOW_TOPLEVEL,
            "decorated",   FALSE,
            "resizable",   FALSE,
            "type-hint",   GDK_WINDOW_TYPE_HINT_DOCK,
            NULL
        )
    );
}

void wintc_taskband_window_toggle_start_menu(
    WinTCTaskbandWindow* taskband
)
{
    wintc_toolbar_start_toggle_menu(
        WINTC_TOOLBAR_START(taskband->uictl_start)
    );
}

//
// CALLBACKS
//
static gboolean on_window_map_event(
    GtkWidget* self,
    WINTC_UNUSED(GdkEventAny* event),
    WINTC_UNUSED(gpointer     user_data)
)
{
    WinTCTaskbandWindow* taskband = WINTC_TASKBAND_WINDOW(self);

    // If we already spawned the toolbars, don't do it again
    //
    if (taskband->toolbars)
    {
        return TRUE;
    }

    // Spawn toolbars
    //
    for (guint i = 0; i < G_N_ELEMENTS(S_LAYOUT); i++)
    {
        switch (S_LAYOUT[i])
        {
            case WINTC_TASKBAND_TOOLBAR_START:
                taskband->uictl_start =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_START,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;

            case WINTC_TASKBAND_TOOLBAR_BUTTONS:
                taskband->uictl_taskbuttons =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_TASK_BUTTONS,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;

            case WINTC_TASKBAND_TOOLBAR_NOTIFICATION_AREA:
                taskband->uictl_notifarea =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_NOTIF_AREA,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;

            case WINTC_TASKBAND_TOOLBAR_QUICK_ACCESS:
            case WINTC_TASKBAND_TOOLBAR_PRE_BUTTONS:
            case WINTC_TASKBAND_TOOLBAR_POST_BUTTONS:
                WINTC_LOG_DEBUG("Not implemented toolbar: %d", S_LAYOUT[i]);
                break;
        }
    }

    return TRUE;
}

