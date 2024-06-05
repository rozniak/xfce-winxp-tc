#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "application.h"
#include "toolbar.h"
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
static void wintc_taskband_window_dispose(
    GObject* object
);

static void wintc_taskband_window_create_toolbar(
    WinTCTaskbandWindow* taskband,
    GType                toolbar_type,
    gboolean             expand
);

static gboolean on_window_map_event(
    GtkWidget*   self,
    GdkEventAny* event,
    gpointer     user_data
);

//
// STATIC DATA
//
static const WinTCTaskbandToolbarId s_layout[] = {
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
    GtkWidget*     main_box;

    GtkWidget*     notification_area;
    GtkWidget*     start_button;
    GtkWidget*     taskbuttons;

    GSList* toolbars;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCTaskbandWindow,
    wintc_taskband_window,
    GTK_TYPE_APPLICATION_WINDOW
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

//
// FORWARD DECLARATIONS
//
static void wintc_taskband_window_dispose(
    GObject* object
)
{
    WinTCTaskbandWindow* wnd = WINTC_TASKBAND_WINDOW(object);

    g_clear_slist(&(wnd->toolbars), g_object_unref);

    (G_OBJECT_CLASS(wintc_taskband_window_parent_class))->dispose(object);
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

//
// PRIVATE FUNCTIONS
//
static void wintc_taskband_window_create_toolbar(
    WinTCTaskbandWindow* taskband,
    GType                toolbar_type,
    gboolean             expand
)
{
    WinTCTaskbandToolbar* toolbar = g_object_new(toolbar_type, NULL);
    GtkWidget*            root    = wintc_taskband_toolbar_get_root_widget(
                                        toolbar
                                    );

    taskband->toolbars =
        g_slist_append(
            taskband->toolbars,
            toolbar
        );

    gtk_box_pack_start(
        GTK_BOX(taskband->main_box),
        root,
        expand,
        expand,
        0
    );

    gtk_widget_show_all(root);
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

    // Spawn toolbars
    //
    for (guint i = 0; i < G_N_ELEMENTS(s_layout); i++)
    {
        switch (s_layout[i])
        {
            case WINTC_TASKBAND_TOOLBAR_START:
                wintc_taskband_window_create_toolbar(
                    taskband,
                    WINTC_TYPE_TOOLBAR_START,
                    FALSE
                );
                break;

            case WINTC_TASKBAND_TOOLBAR_BUTTONS:
                wintc_taskband_window_create_toolbar(
                    taskband,
                    WINTC_TYPE_TOOLBAR_TASK_BUTTONS,
                    TRUE
                );
                break;

            case WINTC_TASKBAND_TOOLBAR_NOTIFICATION_AREA:
                wintc_taskband_window_create_toolbar(
                    taskband,
                    WINTC_TYPE_TOOLBAR_NOTIF_AREA,
                    FALSE
                );
                break;

            case WINTC_TASKBAND_TOOLBAR_QUICK_ACCESS:
            case WINTC_TASKBAND_TOOLBAR_PRE_BUTTONS:
            case WINTC_TASKBAND_TOOLBAR_POST_BUTTONS:
                WINTC_LOG_DEBUG("Not implemented toolbar: %d", s_layout[i]);
                break;
        }
    }

    return TRUE;
}

