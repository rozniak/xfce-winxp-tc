#include <glib.h>
#include <gtk/gtk.h>
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

typedef struct 
{
    WindowManagerSingle *window_manager;
    guint64 timestamp;
} ContextMenuCallbackData;

//
// FORWARD DECLARATIONS
//
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

static gboolean on_window_button_button_released(
    GtkWidget*      self,
    GdkEventButton* event,
    gpointer        user_data
);
static void show_window_context_menu(
    WindowManagerSingle* window_manager,
    GdkEventButton* event
);
static gboolean on_context_menu_restore_clicked(
    GtkWidget* self,
    gpointer user_data
);
static gboolean on_context_menu_minimize_clicked(
    GtkWidget* self,
    gpointer user_data
);
static gboolean on_context_menu_maximize_clicked(
    GtkWidget* self,
    gpointer user_data
);
static gboolean on_context_menu_close_clicked(
    GtkWidget* self,
    gpointer user_data
);
static gboolean maximize_window_callback(
    gpointer user_data
);
static void free_context_menu_callback_data(
    gpointer data,
    GClosure *closure
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
            G_CALLBACK(on_window_button_button_released),
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

static gboolean maximize_window_callback(gpointer user_data)
{
    WinTCWndMgmtWindow* window = (WinTCWndMgmtWindow*) user_data;
    
    wintc_wndmgmt_window_maximize(window);
    
    return G_SOURCE_REMOVE;
}

//
// CALLBACKS
//
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

static gboolean on_window_button_button_released(
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
            show_window_context_menu(
                window_manager,
                event
            );
            break;
        default:
            break;
    }

    return FALSE;
}

static void show_window_context_menu(
    WindowManagerSingle* window_manager,
    GdkEventButton* event
)
{
    GtkWidget *context_menu = gtk_menu_new();
    gtk_menu_set_reserve_toggle_size(GTK_MENU(context_menu), FALSE);

    ContextMenuCallbackData* callback_data = g_new(ContextMenuCallbackData, 1);
    callback_data->window_manager = window_manager;
    callback_data->timestamp = event->time;

    GtkWidget *restore_item = gtk_menu_item_new();
    gtk_widget_set_tooltip_text(restore_item, "Restore Window");


    g_signal_connect_data(
        restore_item, 
        "activate", 
        G_CALLBACK(on_context_menu_restore_clicked), 
        callback_data,
        free_context_menu_callback_data,
        0
    );
    
    GtkWidget *restore_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *restore_icon = gtk_image_new_from_icon_name(
        "window-restore-symbolic",
        GTK_ICON_SIZE_MENU
    );
    gtk_widget_set_margin_end(restore_icon, 10);
    
    GtkWidget *restore_label = gtk_label_new("Restore");
    gtk_label_set_xalign(GTK_LABEL(restore_label), 0.0);

    gtk_box_pack_start(GTK_BOX(restore_box), restore_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(restore_box), restore_label, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(restore_item), restore_box);

    if (
        wintc_wndmgmt_window_is_minimized(window_manager->managed_window)
    )
    {
        gtk_widget_set_sensitive(restore_item, TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(restore_item, FALSE);
    }

    GtkWidget *minimize_item = gtk_menu_item_new();
    gtk_widget_set_tooltip_text(minimize_item, "Minimize Window");


    g_signal_connect_data(
        minimize_item, 
        "activate", 
        G_CALLBACK(on_context_menu_minimize_clicked), 
        callback_data,
        free_context_menu_callback_data,
        0
    );
    
    GtkWidget *minimize_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *minimize_icon = gtk_image_new_from_icon_name(
        "window-minimize-symbolic",
        GTK_ICON_SIZE_MENU
    );
    gtk_widget_set_margin_end(minimize_icon, 10);
    
    GtkWidget *minimize_label = gtk_label_new("Minimize");
    gtk_label_set_xalign(GTK_LABEL(minimize_label), 0.0);

    gtk_box_pack_start(GTK_BOX(minimize_box), minimize_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(minimize_box), minimize_label, TRUE, TRUE, 0);
    
    gtk_container_add(GTK_CONTAINER(minimize_item), minimize_box);


    GtkWidget *maximize_item = gtk_menu_item_new();
    gtk_widget_set_tooltip_text(maximize_item, "Maximize Window");

    g_signal_connect_data(
        maximize_item,
        "activate",
        G_CALLBACK(on_context_menu_maximize_clicked),
        callback_data,
        free_context_menu_callback_data,
        0
    );

    GtkWidget *maximize_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    GtkWidget *maximize_icon = gtk_image_new_from_icon_name(
        "window-maximize-symbolic",
        GTK_ICON_SIZE_MENU
    );
    gtk_widget_set_margin_end(maximize_icon, 10);

    GtkWidget *maximize_label = gtk_label_new("Maximize");
    gtk_label_set_xalign(GTK_LABEL(maximize_label), 0.0);
    

    gtk_box_pack_start(GTK_BOX(maximize_box), maximize_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(maximize_box), maximize_label, TRUE, TRUE, 0);   

    gtk_container_add(GTK_CONTAINER(maximize_item), maximize_box);


    GtkWidget *divider = gtk_separator_menu_item_new();


    GtkWidget *close_item = gtk_menu_item_new();
    gtk_widget_set_tooltip_text(close_item, "close Window");
                
    g_signal_connect_data(
        close_item,
        "activate",
        G_CALLBACK(on_context_menu_close_clicked),
        callback_data,
        free_context_menu_callback_data,
        0
    );      

    GtkWidget *close_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *close_icon = gtk_image_new_from_icon_name(
        "window-close-symbolic",
        GTK_ICON_SIZE_MENU
    );
    gtk_widget_set_margin_end(close_icon, 10);

    GtkWidget *close_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(close_label), "<b>Close</b>");
    gtk_label_set_xalign(GTK_LABEL(close_label), 0.0);

    GtkWidget *hint_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(hint_label), "<b>Alt+F4</b>");
    gtk_label_set_xalign(GTK_LABEL(hint_label), 1.0);


    gtk_box_pack_start(GTK_BOX(close_box), close_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(close_box), close_label, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(close_box), hint_label, FALSE, FALSE, 10);
    
    gtk_container_add(GTK_CONTAINER(close_item), close_box);

    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), restore_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), minimize_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), maximize_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), divider);
    gtk_menu_shell_append(GTK_MENU_SHELL(context_menu), close_item);
    gtk_widget_show_all(context_menu);

    gtk_menu_popup_at_pointer(
        GTK_MENU(context_menu),
        (GdkEvent*) event
    );
}

static gboolean on_context_menu_restore_clicked(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    ContextMenuCallbackData* data = (ContextMenuCallbackData*) user_data;

    wintc_wndmgmt_window_unminimize(data->window_manager->managed_window, data->timestamp);

    return FALSE;
}

static gboolean on_context_menu_minimize_clicked(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    ContextMenuCallbackData* data = (ContextMenuCallbackData*) user_data;

    wintc_wndmgmt_window_minimize(data->window_manager->managed_window);

    return FALSE;
}

static gboolean on_context_menu_maximize_clicked(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    ContextMenuCallbackData* data = (ContextMenuCallbackData*) user_data;

    gboolean is_active = 
        wintc_wndmgmt_screen_get_active_window(data->window_manager->parent_monitor->screen) == 
                                               data->window_manager->managed_window;

    if (is_active)
    {
        wintc_wndmgmt_window_maximize(data->window_manager->managed_window);
    }
    else
    {
       wintc_wndmgmt_window_unminimize(data->window_manager->managed_window, data->timestamp);
       // HACK: Calling maximize immediately after unminimize can cause it not to actually maximize
        g_timeout_add(
            100,  
            maximize_window_callback,
            data->window_manager->managed_window
        );
    }
    return FALSE;
}

static gboolean on_context_menu_close_clicked(
    WINTC_UNUSED(GtkWidget* self),
    gpointer user_data
)
{
    ContextMenuCallbackData *data = (ContextMenuCallbackData*) user_data;

    wintc_wndmgmt_window_close(data->window_manager->managed_window, data->timestamp);

    return FALSE;
}

static void free_context_menu_callback_data(
    gpointer user_data,
    WINTC_UNUSED(GClosure *closure)
)
{
    WINTC_UNUSED(ContextMenuCallbackData* callback_data) = (ContextMenuCallbackData*) user_data;

    g_free(callback_data);
}

