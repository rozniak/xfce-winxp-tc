#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "application.h"
#include "dispproto.h"
#include "window.h"
#include "start/startbutton.h"
#include "systray/notifarea.h"
#include "taskbuttons/taskbuttonbar.h"
#include "taskbuttons/windowmonitor.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskbandWindowPrivate
{
    GtkWidget*     main_box;

    GtkWidget*     notification_area;
    GtkWidget*     start_button;
    GtkWidget*     taskbuttons;
};

struct _WinTCTaskbandWindowClass
{
    GtkApplicationWindowClass __parent__;
};

struct _WinTCTaskbandWindow
{
    GtkApplicationWindow __parent__;

    WinTCTaskbandWindowPrivate* priv;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCTaskbandWindow,
    wintc_taskband_window,
    GTK_TYPE_APPLICATION_WINDOW,
    G_ADD_PRIVATE(WinTCTaskbandWindow)
)

static void wintc_taskband_window_class_init(
    WINTC_UNUSED(WinTCTaskbandWindowClass* klass)
) {}

static void wintc_taskband_window_init(
    WinTCTaskbandWindow* self
)
{
    self->priv = wintc_taskband_window_get_instance_private(self);

    //
    // WINDOW SETUP
    //
    wintc_widget_add_style_class(
        GTK_WIDGET(self),
        "wintc-taskband"
    );

    anchor_taskband_to_bottom(GTK_WINDOW(self));

    //
    // SET UP CHILDREN IN HERE
    // FIXME: Tidy all this stuff up big time! Taskband shouldn't know about
    //        how stuff is built, just ask for a Start button, a taskbar, and
    //        systray -- no implementation details!!
    //

    // Create main container box
    //
    self->priv->main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_add(
        GTK_CONTAINER(self),
        self->priv->main_box
    );

    // Create Start button and menu
    //
    GtkCssProvider* css_start = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css_start,
        "/uk/oddmatics/wintc/taskband/start-menu.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css_start),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    self->priv->start_button = start_button_new();

    gtk_box_pack_start(
        GTK_BOX(self->priv->main_box),
        self->priv->start_button,
        FALSE,
        FALSE,
        0
    );

    // Create task buttons
    //
    self->priv->taskbuttons = taskbutton_bar_new();

    gtk_box_pack_start(
        GTK_BOX(self->priv->main_box),
        self->priv->taskbuttons,
        TRUE,
        TRUE,
        0
    );

    // Create notification area
    //
    self->priv->notification_area = notification_area_new();

    gtk_box_pack_end(
        GTK_BOX(self->priv->main_box),
        self->priv->notification_area,
        FALSE,
        FALSE,
        0
    );
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
            TYPE_WINTC_TASKBAND_WINDOW,
            "application", GTK_APPLICATION(app),
            "type",        GTK_WINDOW_TOPLEVEL,
            "decorated",   FALSE,
            "resizable",   FALSE,
            "type-hint",   GDK_WINDOW_TYPE_HINT_DOCK,
            NULL
        )
    );
}
