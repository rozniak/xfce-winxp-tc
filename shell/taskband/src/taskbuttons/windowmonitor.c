#include <glib.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <pango/pango.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>

#include "windowmonitor.h"

//
// STRUCTURE DEFINITIONS
//
typedef struct _WindowManagerSingle
{
    GtkToggleButton*    button;
    GtkImage*           button_icon;
    GtkLabel*           button_text;
    WinTCWndMgmtWindow* managed_window;
    WindowMonitor*      parent_monitor;
} WindowManagerSingle;

struct _WindowMonitor
{
    GtkContainer*       container;
    WinTCWndMgmtScreen* screen;
    GHashTable*         window_manager_map;
};

typedef struct _ContextMenuCallbackData
{
    WindowManagerSingle* window_manager;
    guint64              timestamp;
} ContextMenuCallbackData;

//
// FORWARD DECLARATIONS
//
static void window_monitor_show_context_menu(
    WindowManagerSingle* window_manager,
    GdkEventButton* event
);
static void window_manager_update_icon(
    WindowManagerSingle* window_manager
);
static void window_manager_update_state(
    WindowManagerSingle* window_manager
);
static void window_manager_update_text(
    WindowManagerSingle* window_manager
);

static void action_restore(
    GSimpleAction *action, 
    GVariant *parameter, 
    gpointer user_data
);
static void action_minimize(
    GSimpleAction *action, 
    GVariant *parameter, 
    gpointer user_data
);
static void action_maximize(
    GSimpleAction *action, 
    GVariant *parameter, 
    gpointer user_data
);
static void action_exit(
    GSimpleAction *action, 
    GVariant *parameter, 
    gpointer user_data
);

static gboolean cb_maximize_window(
    gpointer user_data
);

static void on_active_window_changed(
    WinTCWndMgmtScreen* screen,
    WinTCWndMgmtWindow* previously_active_window,
    gpointer            user_data
);
static void on_window_closed(
    WinTCWndMgmtScreen* screen,
    WinTCWndMgmtWindow* window,
    gpointer            user_data
);
static void on_window_opened(
    WinTCWndMgmtScreen* screen,
    WinTCWndMgmtWindow* window,
    gpointer            user_data
);

static void on_window_icon_changed(
    WinTCWndMgmtWindow* window,
    gpointer            user_data
);
static void on_window_name_changed(
    WinTCWndMgmtWindow* window,
    gpointer            user_data
);
static void on_window_state_changed(
    WinTCWndMgmtWindow* window,
    gint                changed_mask,
    gint                new_state,
    gpointer            user_data
);

static gboolean on_window_button_button_release_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);

//
// STATIC DATA
//
static GActionEntry S_ACTIONS[] = {
    {
        .name           = "exit",
        .activate       = action_exit,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "maximize",
        .activate       = action_maximize,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "minimize",
        .activate       = action_minimize,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    },
    {
        .name           = "restore",
        .activate       = action_restore,
        .parameter_type = NULL,
        .state          = NULL,
        .change_state   = NULL
    }
};

static GtkMenu*             S_WM_MENU       = NULL;
static WindowManagerSingle* S_WM_MENU_CURWM = NULL;

//
// PUBLIC FUNCTIONS
//
void window_monitor_destroy(
    WindowMonitor* monitor
)
{
    g_hash_table_unref(monitor->window_manager_map);
    g_free(monitor);

    wintc_wndmgmt_shutdown();
}

WindowMonitor* window_monitor_init_management(
    GtkContainer* container
)
{
    WindowMonitor* window_monitor = g_new(WindowMonitor, 1);

    window_monitor->container          = container;
    window_monitor->screen             = wintc_wndmgmt_screen_get_default();
    window_monitor->window_manager_map = g_hash_table_new_full(
                                             g_direct_hash,
                                             g_direct_equal,
                                             NULL,
                                             g_free
                                         );

    g_signal_connect(
        window_monitor->screen,
        "active-window-changed",
        G_CALLBACK(on_active_window_changed),
        window_monitor
    );
    g_signal_connect(
        window_monitor->screen,
        "window-closed",
        G_CALLBACK(on_window_closed),
        window_monitor
    );
    g_signal_connect(
        window_monitor->screen,
        "window-opened",
        G_CALLBACK(on_window_opened),
        window_monitor
    );

    return window_monitor;
}

//
// PRIVATE FUNCTIONS
//
static void window_monitor_show_context_menu(
    WindowManagerSingle* window_manager,
    GdkEventButton*      event
)
{
    if (!S_WM_MENU)
    {
        GtkBuilder* builder;
        GMenuModel* menu_model;

        builder =
            gtk_builder_new_from_resource(
                "/uk/oddmatics/wintc/taskband/taskbutton-ctx-menu.ui"
            );

        wintc_builder_get_objects(
            builder,
            "context-menu", &S_WM_MENU,
            "menu-model",   &menu_model,
            NULL
        );

        g_object_ref(S_WM_MENU);

        gtk_menu_shell_bind_model(
            GTK_MENU_SHELL(S_WM_MENU),
            menu_model,
            NULL,
            TRUE
        );

        g_object_unref(builder);

        // Set up actions
        //
        GSimpleActionGroup* action_group = g_simple_action_group_new();

        g_action_map_add_action_entries(
            G_ACTION_MAP(action_group),
            S_ACTIONS,
            G_N_ELEMENTS(S_ACTIONS),
            &S_WM_MENU_CURWM
        );

        gtk_widget_insert_action_group(
            GTK_WIDGET(S_WM_MENU),
            "window-manager",
            G_ACTION_GROUP(action_group)
        );
    }

    // Update action states for this window
    //
    GActionGroup* wm_action_group =
        gtk_widget_get_action_group(
            GTK_WIDGET(S_WM_MENU),
            "window-manager"
        );

    GSimpleAction* maximize_action = G_SIMPLE_ACTION(
                                         g_action_map_lookup_action(
                                             G_ACTION_MAP(wm_action_group),
                                            "maximize"
                                         )
                                     );
    GSimpleAction* minimize_action = G_SIMPLE_ACTION(
                                         g_action_map_lookup_action(
                                             G_ACTION_MAP(wm_action_group),
                                             "minimize"
                                         )
                                     );
    GSimpleAction* restore_action  = G_SIMPLE_ACTION(
                                         g_action_map_lookup_action(
                                             G_ACTION_MAP(wm_action_group),
                                             "restore"
                                         )
                                     );

    gboolean is_max =
        wintc_wndmgmt_window_is_maximized(window_manager->managed_window);
    gboolean is_min =
        wintc_wndmgmt_window_is_minimized(window_manager->managed_window);

    g_simple_action_set_enabled(
        maximize_action,
        !is_max
    );
    g_simple_action_set_enabled(
        minimize_action,
        !is_min
    );
    g_simple_action_set_enabled(
        restore_action,
        is_min || is_max
    );

    // Update style classes
    //
    if (is_min)
    {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(GTK_WIDGET(S_WM_MENU)),
            "ctx-menu-focus-restore"
        );
    }
    else
    {
        gtk_style_context_add_class(
            gtk_widget_get_style_context(GTK_WIDGET(S_WM_MENU)),
            "ctx-menu-focus-exit"
        );
    }

    // Popup!
    //
    S_WM_MENU_CURWM = window_manager;

    gtk_menu_popup_at_pointer(
        S_WM_MENU,
        (GdkEvent*) event
    );
}

static void window_manager_update_icon(
    WindowManagerSingle* window_manager
)
{
    GdkPixbuf* icon;

    if (window_manager->button == NULL)
    {
        return;
    }

    icon = wintc_wndmgmt_window_get_mini_icon(window_manager->managed_window);

    gtk_image_set_from_pixbuf(
        window_manager->button_icon,
        icon
    );

    g_object_unref(icon);
}

static void window_manager_update_state(
    WindowManagerSingle* window_manager
)
{
    GtkBox*  button_box;
    gboolean skip_tasklist;

    skip_tasklist =
        wintc_wndmgmt_window_is_skip_tasklist(window_manager->managed_window);

    if (skip_tasklist && window_manager->button != NULL)
    {
        gtk_widget_destroy(GTK_WIDGET(window_manager->button));

        window_manager->button      = NULL;
        window_manager->button_icon = NULL;
        window_manager->button_text = NULL;
    }
    else if (!skip_tasklist && window_manager->button == NULL)
    {
        button_box                  = GTK_BOX(
                                          gtk_box_new(
                                              GTK_ORIENTATION_HORIZONTAL,
                                              0
                                          )
                                      );
        window_manager->button      = GTK_TOGGLE_BUTTON(
                                          gtk_toggle_button_new()
                                      );
        window_manager->button_icon = GTK_IMAGE(gtk_image_new());
        window_manager->button_text = GTK_LABEL(gtk_label_new(NULL));

        window_manager_update_icon(window_manager);
        window_manager_update_text(window_manager);

        gtk_widget_set_halign(
            GTK_WIDGET(window_manager->button_text),
            GTK_ALIGN_START
        );
        gtk_label_set_ellipsize(
            window_manager->button_text,
            PANGO_ELLIPSIZE_END
        );

        g_signal_connect(
            window_manager->button,
            "button-release-event",
            G_CALLBACK(on_window_button_button_release_event),
            window_manager
        );

        gtk_box_pack_start(
            button_box,
            GTK_WIDGET(window_manager->button_icon),
            FALSE,
            FALSE,
            0
        );
        gtk_box_pack_start(
            button_box,
            GTK_WIDGET(window_manager->button_text),
            TRUE,
            TRUE,
            0
        );

        gtk_container_add(
            GTK_CONTAINER(window_manager->button),
            GTK_WIDGET(button_box)
        );

        gtk_container_add(
            window_manager->parent_monitor->container,
            GTK_WIDGET(window_manager->button)
        );

        gtk_widget_show_all(GTK_WIDGET(window_manager->button));
    }
}

static void window_manager_update_text(
    WindowManagerSingle* window_manager
)
{
    const gchar* new_text =
        wintc_wndmgmt_window_get_name(window_manager->managed_window);

    gtk_label_set_text(
        window_manager->button_text,
        new_text
    );
    gtk_widget_set_tooltip_text(
        GTK_WIDGET(window_manager->button),
        new_text
    );
}

//
// CALLBACKS
//
static void action_exit(
    WINTC_UNUSED(GSimpleAction *action),
    WINTC_UNUSED(GVariant *parameter),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager =
        *((WindowManagerSingle**) user_data);

    wintc_wndmgmt_window_close(
        window_manager->managed_window,
        gtk_get_current_event_time()
    );
}

static void action_maximize(
    WINTC_UNUSED(GSimpleAction *action),
    WINTC_UNUSED(GVariant *parameter),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager =
        *((WindowManagerSingle**) user_data);

    gboolean is_active = 
        wintc_wndmgmt_screen_get_active_window(
            window_manager->parent_monitor->screen
        ) == window_manager->managed_window;

    if (is_active)
    {
        wintc_wndmgmt_window_maximize(
            window_manager->managed_window
        );
    }
    else
    {
        wintc_wndmgmt_window_unminimize(
            window_manager->managed_window,
            gtk_get_current_event_time()
        );

        // HACK: Calling maximize immediately after unminimize can cause it
        //       to not actually maximize
        //
        g_timeout_add(
            100,  
            cb_maximize_window,
            window_manager->managed_window
        );
    }
}

static void action_minimize(
    WINTC_UNUSED(GSimpleAction *action),
    WINTC_UNUSED(GVariant *parameter),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager =
        *((WindowManagerSingle**) user_data);

    wintc_wndmgmt_window_minimize(
        window_manager->managed_window
    );
}

static void action_restore(
    WINTC_UNUSED(GSimpleAction *action),
    WINTC_UNUSED(GVariant *parameter),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager =
        *((WindowManagerSingle**) user_data);

    if (
        wintc_wndmgmt_window_is_minimized(
            window_manager->managed_window
        )
    )
    {
        wintc_wndmgmt_window_unminimize(
            window_manager->managed_window,
            gtk_get_current_event_time()
        );
    }
    else
    {
        wintc_wndmgmt_window_unmaximize(
            window_manager->managed_window
        );
    }
}

static gboolean cb_maximize_window(
    gpointer user_data
)
{
    WinTCWndMgmtWindow* window = (WinTCWndMgmtWindow*) user_data;
    
    wintc_wndmgmt_window_maximize(window);
    
    return G_SOURCE_REMOVE;
}

static void on_active_window_changed(
    WINTC_UNUSED(WinTCWndMgmtScreen* screen),
    WinTCWndMgmtWindow* previously_active_window,
    gpointer            user_data
)
{
    WinTCWndMgmtWindow*  active_window;
    WindowManagerSingle* window_manager_old;
    WindowManagerSingle* window_manager_new;
    WindowMonitor*       window_monitor = (WindowMonitor*) user_data;

    active_window =
        wintc_wndmgmt_screen_get_active_window(window_monitor->screen);

    if (previously_active_window != NULL)
    {
        window_manager_old =
            g_hash_table_lookup(
                window_monitor->window_manager_map,
                previously_active_window
            );

        if (
            window_manager_old         != NULL &&
            window_manager_old->button != NULL
        )
        {
            gtk_toggle_button_set_active(
                window_manager_old->button,
                FALSE
            );
        }
    }

    if (active_window != NULL)
    {
        window_manager_new =
            g_hash_table_lookup(
                window_monitor->window_manager_map,
                active_window
            );

        if (
            window_manager_new         != NULL &&
            window_manager_new->button != NULL
        )
        {
            gtk_toggle_button_set_active(
                window_manager_new->button,
                TRUE
            );
        }
    }
}

static void on_window_closed(
    WINTC_UNUSED(WinTCWndMgmtScreen* screen),
    WinTCWndMgmtWindow* window,
    gpointer            user_data
)
{
    WindowManagerSingle* window_manager;
    WindowMonitor*       window_monitor = (WindowMonitor*) user_data;

    window_manager =
        g_hash_table_lookup(
            window_monitor->window_manager_map,
            window
        );

    if (window_manager == NULL)
    {
        // Unusual... we're not managing this window? oh well
        //
        return;
    }

    if (window_manager->button != NULL)
    {
        gtk_widget_destroy(GTK_WIDGET(window_manager->button));
    }

    g_hash_table_remove(window_monitor->window_manager_map, window);
}

static void on_window_opened(
    WINTC_UNUSED(WinTCWndMgmtScreen* screen),
    WinTCWndMgmtWindow* window,
    gpointer            user_data
)
{
    WindowManagerSingle* window_manager = g_new(WindowManagerSingle, 1);
    WindowMonitor*       window_monitor = (WindowMonitor*) user_data;

    window_manager->button               = NULL;
    window_manager->button_icon          = NULL;
    window_manager->button_text          = NULL;
    window_manager->managed_window       = window;
    window_manager->parent_monitor       = window_monitor;

    g_hash_table_insert(
        window_monitor->window_manager_map,
        window,
        window_manager
    );

    window_manager_update_state(window_manager);

    g_signal_connect(
        window,
        "icon-changed",
        G_CALLBACK(on_window_icon_changed),
        window_manager
    );
    g_signal_connect(
        window,
        "name-changed",
        G_CALLBACK(on_window_name_changed),
        window_manager
    );
    g_signal_connect(
        window,
        "state-changed",
        G_CALLBACK(on_window_state_changed),
        window_manager
    );
}

static void on_window_icon_changed(
    WINTC_UNUSED(WinTCWndMgmtWindow* window),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_icon(window_manager);
}

static void on_window_name_changed(
    WINTC_UNUSED(WinTCWndMgmtWindow* window),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_text(window_manager);
}

static void on_window_state_changed(
    WINTC_UNUSED(WinTCWndMgmtWindow* window),
    WINTC_UNUSED(gint changed_mask),
    WINTC_UNUSED(gint new_state),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_state(window_manager);
}

static gboolean on_window_button_button_release_event(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
)
{
    GtkToggleButton*     toggle_button  = GTK_TOGGLE_BUTTON(self);
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    switch (event->button) {
        case GDK_BUTTON_PRIMARY:
            if (gtk_toggle_button_get_active(toggle_button))
            {
                wintc_wndmgmt_window_minimize(
                    window_manager->managed_window
                );
            }
            else
            {
                wintc_wndmgmt_window_unminimize(
                    window_manager->managed_window,
                    (guint64) event->time
                );
            }

            break;

        case GDK_BUTTON_SECONDARY:
            // Match XP behavior: if the window isn't minimized, display it on
            // top
            //
            if (
                !wintc_wndmgmt_window_is_minimized(
                    window_manager->managed_window
                )
            )
            {
                wintc_wndmgmt_window_unminimize(
                    window_manager->managed_window,
                    (guint64) event->time
                );
            }

            window_monitor_show_context_menu(
                window_manager,
                event
            );

            break;

        default: break;
    }

    return FALSE;
}
