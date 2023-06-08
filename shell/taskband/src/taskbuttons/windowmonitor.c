#include <glib.h>
#include <gtk/gtk.h>
#include <pango/pango.h>
#include <wintc-comgtk.h>

#include "../dispproto.h"
#include "windowmonitor.h"

//
// STRUCTURE DEFINITIONS
//
typedef struct _WindowManagerSingle
{
    GtkToggleButton* button;
    GtkImage*        button_icon;
    gboolean         button_synchronizing;
    GtkLabel*        button_text;
    WndMgmtWindow*   managed_window;
    WindowMonitor*   parent_monitor;
} WindowManagerSingle;

struct _WindowMonitor
{
    GtkContainer*  container;
    WndMgmtScreen* screen;
    GHashTable*    window_manager_map;
};

//
// FORWARD DECLARATIONS
//
static void on_active_window_changed(
    WndMgmtScreen* screen,
    WndMgmtWindow* previously_active_window,
    gpointer       user_data
);
static void on_window_closed(
    WndMgmtScreen* screen,
    WndMgmtWindow* window,
    gpointer       user_data
);
static void on_window_opened(
    WndMgmtScreen* screen,
    WndMgmtWindow* window,
    gpointer       user_data
);

static void on_window_icon_changed(
    WndMgmtWindow* window,
    gpointer       user_data
);
static void on_window_name_changed(
    WndMgmtWindow* window,
    gpointer       user_data
);
static void on_window_state_changed(
    WndMgmtWindow* window,
    gint           changed_mask,
    gint           new_state,
    gpointer       user_data
);

static void on_window_button_toggled(
    GtkToggleButton* button,
    gpointer         user_data
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
WindowMonitor* window_monitor_init_management(
    GtkContainer* container
)
{
    WindowMonitor* window_monitor = g_new(WindowMonitor, 1);

    window_monitor->container          = container;
    window_monitor->screen             = wndmgmt_screen_get_default();
    window_monitor->window_manager_map = g_hash_table_new(
                                             g_direct_hash,
                                             g_direct_equal
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
    if (window_manager->button == NULL)
    {
        return;
    }

    gtk_image_set_from_pixbuf(
        window_manager->button_icon,
        wndmgmt_window_get_mini_icon(
            window_manager->managed_window
        )
    );
}

static void window_manager_update_state(
    WindowManagerSingle* window_manager
)
{
    GtkBox*  button_box;
    gboolean skip_tasklist;

    skip_tasklist =
        wndmgmt_window_is_skip_tasklist(window_manager->managed_window);

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
            "toggled",
            G_CALLBACK(on_window_button_toggled),
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
        wndmgmt_window_get_name(window_manager->managed_window);

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
static void on_active_window_changed(
    WINTC_UNUSED(WndMgmtScreen* screen),
    WndMgmtWindow* previously_active_window,
    gpointer       user_data
)
{
    WndMgmtWindow*       active_window;
    WindowManagerSingle* window_manager_old;
    WindowManagerSingle* window_manager_new;
    WindowMonitor*       window_monitor = (WindowMonitor*) user_data;

    active_window = wndmgmt_screen_get_active_window(window_monitor->screen);

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
            window_manager_old->button_synchronizing = TRUE;

            gtk_toggle_button_set_active(
                window_manager_old->button,
                FALSE
            );

            window_manager_old->button_synchronizing = FALSE;
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
            window_manager_new->button_synchronizing = TRUE;

            gtk_toggle_button_set_active(
                window_manager_new->button,
                TRUE
            );

            window_manager_new->button_synchronizing = FALSE;
        }
    }
}

static void on_window_closed(
    WINTC_UNUSED(WndMgmtScreen* screen),
    WndMgmtWindow* window,
    gpointer       user_data
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

    g_free(window_manager);
    g_hash_table_remove(window_monitor->window_manager_map, window);
}

static void on_window_opened(
    WINTC_UNUSED(WndMgmtScreen* screen),
    WndMgmtWindow* window,
    gpointer       user_data
)
{
    WindowManagerSingle* window_manager = g_new(WindowManagerSingle, 1);
    WindowMonitor*       window_monitor = (WindowMonitor*) user_data;

    window_manager->button               = NULL;
    window_manager->button_icon          = NULL;
    window_manager->button_synchronizing = FALSE;
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
    WINTC_UNUSED(WndMgmtWindow* window),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_icon(window_manager);
}

static void on_window_name_changed(
    WINTC_UNUSED(WndMgmtWindow* window),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_text(window_manager);
}

static void on_window_state_changed(
    WINTC_UNUSED(WndMgmtWindow* window),
    WINTC_UNUSED(gint changed_mask),
    WINTC_UNUSED(gint new_state),
    gpointer user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    window_manager_update_state(window_manager);
}

static void on_window_button_toggled(
    GtkToggleButton* button,
    gpointer         user_data
)
{
    WindowManagerSingle* window_manager = (WindowManagerSingle*) user_data;

    if (window_manager->button_synchronizing)
    {
        return;
    }

    if (gtk_toggle_button_get_active(button))
    {
        wndmgmt_window_unminimize(window_manager->managed_window);
    }
    else
    {
        wndmgmt_window_minimize(window_manager->managed_window);
    }
}
