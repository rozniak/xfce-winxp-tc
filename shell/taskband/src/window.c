#include <gdk/gdk.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/exec.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>

#include "application.h"
#include "intapi.h"
#include "window.h"
#include "qaccess/toolbar.h"
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
static void wintc_taskband_window_ishext_shell_window_interface_init(
    WinTCIShextShellWindowInterface* iface
);
static void wintc_taskband_window_ishext_ui_host_interface_init(
    WinTCIShextUIHostInterface* iface
);

static void wintc_taskband_window_dispose(
    GObject* object
);

static void wintc_taskband_window_get_location(
    WinTCIShextShellWindow* shwnd,
    WinTCShextPathInfo*     path_info
);
static WinTCShextBrowserMode wintc_taskband_window_get_mode(
    WinTCIShextShellWindow* shwnd
);
static gboolean wintc_taskband_window_navigate(
    WinTCIShextShellWindow*   shwnd,
    const WinTCShextPathInfo* path_info,
    GError**                  error
);
static gboolean wintc_taskband_window_navigate_simple(
    WinTCIShextShellWindow* shwnd,
    const gchar*            path,
    GError**                error
);

static GtkWidget* wintc_taskband_window_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
);

static gboolean on_window_button_release_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
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

static GtkWidget* S_CTX_MENU = NULL;

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _WinTCTaskbandWindow
{
    WinTCDpaPanelWindow __parent__;

    // UI
    //
    GtkWidget* main_box;

    GSList* toolbars;

    GHashTable* map_lib_to_toolbar_info;
    GtkWidget*  latest_toolbar;

    WinTCShextUIController* uictl_qaccess;
    WinTCShextUIController* uictl_notifarea;
    WinTCShextUIController* uictl_start;
    WinTCShextUIController* uictl_taskbuttons;

    GtkWidget* box_pre;
    GtkWidget* box_post;
};

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    WinTCTaskbandWindow,
    wintc_taskband_window,
    WINTC_TYPE_DPA_PANEL_WINDOW,
    G_IMPLEMENT_INTERFACE(
        WINTC_TYPE_ISHEXT_SHELL_WINDOW,
        wintc_taskband_window_ishext_shell_window_interface_init
    )
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

    // Create context menu
    //
    GtkBuilder* builder;
    GMenuModel* menu_model;

    builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/taskband-ctx-menu.ui"
        );

    wintc_builder_get_objects(
        builder,
        "context-menu", &S_CTX_MENU,
        "menu-model",   &menu_model,
        NULL
    );

    g_object_ref(S_CTX_MENU);

    gtk_menu_shell_bind_model(
        GTK_MENU_SHELL(S_CTX_MENU),
        menu_model,
        NULL,
        TRUE
    );

    g_object_unref(builder);
}

static void wintc_taskband_window_init(
    WinTCTaskbandWindow* self
)
{
    GError* error = NULL;

    self->map_lib_to_toolbar_info =
        g_hash_table_new_full(
            g_str_hash,
            g_str_equal,
            NULL,
            (GDestroyNotify) wintc_shext_toolbar_info_free
        );

    // Init toolbars
    //
    GList* list_toolbars =
        wintc_shext_ext_get_toolbar_apis(
            WINTC_SHEXT_SHWND_KIND_CHROME
        );

    for (GList* iter = list_toolbars; iter; iter = iter->next)
    {
        WinTCShextToolbarInfo* toolbar_info =
            (WinTCShextToolbarInfo*) iter->data;

        if (!toolbar_info->fn_shext(self, &error))
        {
            wintc_log_error_and_clear(&error);
            continue;
        }

        if (!(self->latest_toolbar))
        {
            g_warning(
                "taskband: %s was loaded but didn't create a toolbar",
                toolbar_info->soname
            );

            continue;
        }

        toolbar_info->toolbar = g_steal_pointer(&(self->latest_toolbar));

        g_hash_table_insert(
            self->map_lib_to_toolbar_info,
            toolbar_info->soname,
            g_steal_pointer(&(iter->data)) // toolbar_info
        );
    }

    g_clear_list(
        &list_toolbars,
        (GDestroyNotify) wintc_shext_toolbar_info_free
    );

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

    // Create main container box
    //
    self->main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_container_add(
        GTK_CONTAINER(self),
        self->main_box
    );

    g_signal_connect(
        self,
        "button-release-event",
        G_CALLBACK(on_window_button_release_event),
        NULL
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

static void wintc_taskband_window_ishext_shell_window_interface_init(
    WinTCIShextShellWindowInterface* iface
)
{
    iface->get_location    = wintc_taskband_window_get_location;
    iface->get_mode        = wintc_taskband_window_get_mode;
    iface->navigate        = wintc_taskband_window_navigate;
    iface->navigate_simple = wintc_taskband_window_navigate_simple;
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

    g_clear_object(&(taskband->uictl_qaccess));
    g_clear_object(&(taskband->uictl_notifarea));
    g_clear_object(&(taskband->uictl_start));
    g_clear_object(&(taskband->uictl_taskbuttons));

    if (taskband->map_lib_to_toolbar_info)
    {
        g_hash_table_destroy(
            g_steal_pointer(&(taskband->map_lib_to_toolbar_info))
        );
    }

    (G_OBJECT_CLASS(wintc_taskband_window_parent_class))->dispose(object);
}

//
// INTERFACE METHODS (WinTCIShextShellWindow)
//
static void wintc_taskband_window_get_location(
    WINTC_UNUSED(WinTCIShextShellWindow* shwnd),
    WINTC_UNUSED(WinTCShextPathInfo* path_info)
) {}

static WinTCShextBrowserMode wintc_taskband_window_get_mode(
    WINTC_UNUSED(WinTCIShextShellWindow* shwnd)
)
{
    return WINTC_SHEXT_BROWSER_MODE_INVALID;
}

static gboolean wintc_taskband_window_navigate(
    WinTCIShextShellWindow*   shwnd,
    const WinTCShextPathInfo* path_info,
    GError**                  error
)
{
    gchar*   path = wintc_shext_path_info_get_as_single_path(path_info);
    gboolean ret  = wintc_taskband_window_navigate_simple(shwnd, path, error);

    g_free(path);

    return ret;
}

static gboolean wintc_taskband_window_navigate_simple(
    WINTC_UNUSED(WinTCIShextShellWindow* shwnd),
    const gchar* path,
    GError**     error
)
{
    gchar*   cmd = g_strdup_printf("explorer %s", path);
    gboolean ret = wintc_launch_command(cmd, error);

    g_free(cmd);

    return ret;
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

    switch (ext_id)
    {
        // Shell toolbars - ctx coming in is the toolbar created by the shell
        // extension, we just host it
        //
        case WINTC_SHEXT_SHWND_HOSTEXT_TOOLBAR:
            if (
                expected_type != GTK_TYPE_TOOLBAR ||
                !GTK_IS_TOOLBAR(ctx)
            )
            {
                g_critical(
                    "%s",
                    "taskband: toolbar requested, but toolbar not expected"
                );
                return NULL;
            }

            if (taskband->latest_toolbar)
            {
                g_critical(
                    "%s",
                    "taskband: attempted to create more than one toolbar"
                );
                return NULL;
            }

            taskband->latest_toolbar = GTK_WIDGET(ctx);
            g_object_ref(taskband->latest_toolbar);

            return ctx;

        // Internal toolbars
        //
        case WINTC_TASKBAND_HOSTEXT_TOOLBAR:
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

        default:
            g_critical("taskband: invalid extension requested: %d", ext_id);
            return NULL;
    }
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
static gboolean on_window_button_release_event(
    WINTC_UNUSED(GtkWidget* self),
    GdkEventButton* event,
    WINTC_UNUSED(gpointer user_data)
)
{
    if (event->button != GDK_BUTTON_SECONDARY)
    {
        return FALSE;
    }

    gtk_menu_popup_at_pointer(
        GTK_MENU(S_CTX_MENU),
        (GdkEvent*) event
    );

    return TRUE;
}

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

            case WINTC_TASKBAND_TOOLBAR_QUICK_ACCESS:
                taskband->uictl_qaccess =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_QUICK_ACCESS,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;

            case WINTC_TASKBAND_TOOLBAR_PRE_BUTTONS:
                taskband->box_pre =
                    gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

                gtk_widget_show_all(taskband->box_pre);

                break;

            case WINTC_TASKBAND_TOOLBAR_BUTTONS:
                taskband->uictl_taskbuttons =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_TASK_BUTTONS,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;

            case WINTC_TASKBAND_TOOLBAR_POST_BUTTONS:
                taskband->box_post =
                    gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

                gtk_widget_show_all(taskband->box_post);

                break;

            case WINTC_TASKBAND_TOOLBAR_NOTIFICATION_AREA:
                taskband->uictl_notifarea =
                    wintc_shext_ui_controller_new_from_type(
                        WINTC_TYPE_TOOLBAR_NOTIF_AREA,
                        WINTC_ISHEXT_UI_HOST(taskband)
                    );

                break;
        }
    }

    WINTC_LOG_DEBUG("taskband: toolbar init complete");

    return TRUE;
}

