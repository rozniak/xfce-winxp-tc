#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>
#include <unistd.h>

#include "action.h"
#include "placeslist.h"
#include "programslist.h"
#include "startmenu.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartMenuPrivate
{
    StartMenu* menu;

    GtkWidget* main_box;
    GtkWidget* places_list;
    GtkWidget* programs_list;
};

struct _StartMenuClass
{
    GtkWindowClass __parent__;
};

struct _StartMenu
{
    GtkWindow __parent__;

    StartMenuPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
static void start_menu_finalize(
    GObject* object
);

static void create_logoffpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
);
static void create_places_structure(
    StartMenu* start_menu,
    GtkBox*    box
);
static void create_programs_structure(
    StartMenu* start_menu,
    GtkBox*    box
);
static void create_taskcolumns_structure(
    StartMenu* start_menu,
    GtkBox*    box
);
static void create_userpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
);

static void on_action_button_clicked(
    GtkButton* button,
    gpointer   user_data
);
static gboolean on_focus_out(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
);
static void on_selection_done(
    GtkWidget* widget,
    StartMenu* start_menu
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE_WITH_CODE(
    StartMenu,
    start_menu,
    GTK_TYPE_WINDOW,
    G_ADD_PRIVATE(StartMenu)
);

static void start_menu_class_init(
    StartMenuClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = start_menu_finalize;
}

static void start_menu_init(
    StartMenu* self
)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self, TYPE_START_MENU, StartMenuPrivate);

    // Set up hints
    //
    gtk_window_set_type_hint(GTK_WINDOW(self), GDK_WINDOW_TYPE_HINT_POPUP_MENU);
    gtk_window_set_resizable(GTK_WINDOW(self), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(self), TRUE);
    gtk_window_stick(GTK_WINDOW(self));
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(self), TRUE);
    gtk_window_set_title(GTK_WINDOW(self), "Start menu");
    gtk_widget_set_events(GTK_WIDGET(self), GDK_FOCUS_CHANGE_MASK);

    // Set up signals
    //
    g_signal_connect(
        G_OBJECT(self),
        "delete-event",
        G_CALLBACK(gtk_widget_hide_on_delete),
        NULL
    );
    g_signal_connect(
        G_OBJECT(self),
        "focus-out-event",
        G_CALLBACK(on_focus_out),
        NULL
    );

    // Set titlebar to the fake titlebar box
    //
    GtkWidget* fake_titlebar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_window_set_titlebar(GTK_WINDOW(self), fake_titlebar);

    // Set up structure
    //
    self->priv->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    create_userpane_structure(self, GTK_BOX(self->priv->main_box));
    create_taskcolumns_structure(self, GTK_BOX(self->priv->main_box));
    create_logoffpane_structure(self, GTK_BOX(self->priv->main_box));

    gtk_container_add(GTK_CONTAINER(self), self->priv->main_box);

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(self), "xp-start-menu");

    gtk_widget_show_all(self->priv->main_box);
}

//
// FINALIZE
//
static void start_menu_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(start_menu_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
void start_menu_get_popup_position(
    GtkWidget*       menu,
    XfcePanelPlugin* plugin,
    GtkWidget*       widget,
    gint*            x,
    gint*            y
)
{
    xfce_panel_plugin_position_widget(
        plugin,
        (START_MENU(menu))->priv->main_box,
        widget,
        x,
        y
    );
}

//
// PRIVATE FUNCTIONS
//
static void create_logoffpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    GtkWidget* logoffpane_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Log off button
    //
    GtkWidget* logoff_button = gtk_button_new();
    GtkWidget* logoff_box    = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* logoff_icon   = gtk_image_new_from_icon_name(
                                   "system-log-out",
                                   GTK_ICON_SIZE_LARGE_TOOLBAR
                               );
    GtkWidget* logoff_label  = gtk_label_new("Log Off"); // FIXME: Use localized string

    gtk_box_pack_start(GTK_BOX(logoff_box), logoff_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(logoff_box), logoff_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(logoff_button), logoff_box);

    g_signal_connect(
        G_OBJECT(logoff_button),
        "clicked",
        G_CALLBACK(on_action_button_clicked),
        GINT_TO_POINTER(XP_ACTION_LOGOFF)
    );

    // FIXME: Localize
    //
    gtk_widget_set_tooltip_text(
        logoff_button,
        "Provides options for closing your programs and logging off, or for leaving your programs running and switching to another user."
    );

    // Shut down button
    //
    GtkWidget* shutdown_button = gtk_button_new();
    GtkWidget* shutdown_box    = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* shutdown_icon   = gtk_image_new_from_icon_name(
                                     "system-shutdown",
                                     GTK_ICON_SIZE_LARGE_TOOLBAR
                                 );
    GtkWidget* shutdown_label  = gtk_label_new("Turn Off Computer"); // FIXME: Localize

    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_icon, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(shutdown_button), shutdown_box);

    g_signal_connect(
        G_OBJECT(shutdown_button),
        "clicked",
        G_CALLBACK(on_action_button_clicked),
        GINT_TO_POINTER(XP_ACTION_SHUTDOWN)
    );

    // FIXME: Localize
    //
    gtk_widget_set_tooltip_text(
        shutdown_button,
        "Provides options for turning off or restarting your computer, or for activating Stand By or Hibernate modes."
    );

    // Pack box
    //
    gtk_box_pack_end(GTK_BOX(logoffpane_box), shutdown_button, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(logoffpane_box), logoff_button, FALSE, FALSE, 0);
    gtk_box_pack_start(box, logoffpane_box, FALSE, FALSE, 0);

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(logoffpane_box), "xp-start-logoffpane");
}

static void create_places_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    GtkWidget* places_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create menu
    //
    GtkWidget* places_list = GTK_WIDGET(g_object_new(TYPE_PLACES_LIST, NULL));

    // Pack box
    //
    gtk_box_pack_start(GTK_BOX(places_box), places_list, TRUE, TRUE, 0);
    gtk_box_pack_start(box,                 places_box,  TRUE, TRUE, 0);

    // Set up signals
    //
    GList* menu_children =
        gtk_container_get_children(
            GTK_CONTAINER(places_list)
        );

    connect_widget_list_signals(
        menu_children,
        "enter-notify-event",
        G_CALLBACK(menu_shell_select_on_enter),
        places_list
    );
    connect_widget_list_signals(
        menu_children,
        "leave-notify-event",
        G_CALLBACK(menu_shell_deselect_on_leave),
        places_list
    );

    g_list_free(g_steal_pointer(&menu_children));

    g_signal_connect(
        places_list,
        "selection-done",
        G_CALLBACK(on_selection_done),
        start_menu
    );

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(places_box), "xp-start-places-column");

    // Connect to instance
    //
    start_menu->priv->places_list = places_list;
}

static void create_programs_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    GtkWidget* programs_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Create menu
    //
    GtkWidget* programs_list = GTK_WIDGET(g_object_new(TYPE_PROGRAMS_LIST, NULL));

    // Pack box
    // 
    gtk_box_pack_start(GTK_BOX(programs_box), programs_list, TRUE, TRUE, 0);
    gtk_box_pack_start(box,                   programs_box,  TRUE, TRUE, 0);

    // Set up signals
    //
    GList* menu_children =
        gtk_container_get_children(
            GTK_CONTAINER(programs_list)
        );

    connect_widget_list_signals(
        menu_children,
        "enter-notify-event",
        G_CALLBACK(menu_shell_select_on_enter),
        programs_list
    );
    connect_widget_list_signals(
        menu_children,
        "leave-notify-event",
        G_CALLBACK(menu_shell_deselect_on_leave),
        programs_list
    );

    g_list_free(g_steal_pointer(&menu_children));

    g_signal_connect(
        programs_list,
        "selection-done",
        G_CALLBACK(on_selection_done),
        start_menu
    );

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(programs_box), "xp-start-programs-column");

    // Connect to instance
    //
    start_menu->priv->programs_list = programs_list;
}

static void create_taskcolumns_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    // Create structure
    //
    GtkWidget* columns_box  = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    create_programs_structure(start_menu, GTK_BOX(columns_box));
    create_places_structure(start_menu, GTK_BOX(columns_box));

    gtk_box_pack_start(box, columns_box, TRUE, TRUE, 0);

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(columns_box), "xp-start-columns");
}

static void create_userpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    GtkWidget* userpane_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // User profile picture
    //
    // NOTE:
    //     We store the image widget inside an events box so that in future we can
    //     make use of the click event to change the user profile picture (this is
    //     what Windows XP does)
    //
    //     The events box is in the larger outside box so it can be styled with a
    //     frame via CSS
    //
    //
    // TODO: For now, we just use a placeholder image for the user's display picture
    //       -- perhaps there is a freedesktop.org standard for this we can use
    //
    // FIXME: Shift 48, 48px size out to a constant somewhere
    //
    GError*    load_error    = NULL;
    GdkPixbuf* pic           = gdk_pixbuf_new_from_file(
                                   "/usr/share/winxp/shell-res/fpo-userpic.png",
                                   &load_error
                               );
    GtkWidget* pic_box       = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* pic_event_box = gtk_event_box_new();
    GtkWidget* pic_image     = gtk_image_new();

    if (pic != NULL)
    {
        GdkPixbuf* scaled_pic =
            gdk_pixbuf_scale_simple(
                pic,
                48,
                48,
                GDK_INTERP_BILINEAR
            );

        g_clear_object(&pic);

        gtk_image_set_from_pixbuf(GTK_IMAGE(pic_image), scaled_pic);
    }
    else
    {
        report_g_error_and_clear(&load_error);
        gtk_widget_set_size_request(pic_image, 48, 48);
    }

    gtk_container_add(GTK_CONTAINER(pic_event_box), pic_image);
    gtk_container_add(GTK_CONTAINER(pic_box),       pic_event_box);

    // Username display
    //
    GtkWidget* username_label = gtk_label_new(getlogin());

    // Construct box
    //
    gtk_box_pack_start(GTK_BOX(userpane_box), pic_box,        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(userpane_box), username_label, FALSE, FALSE, 0);

    gtk_box_pack_start(box, userpane_box, FALSE, FALSE, 0);

    // Add style class
    //
    gtk_widget_add_style_class(userpane_box, "xp-start-userpane");
}

//
// CALLBACKS
//
static void on_action_button_clicked(
    GtkButton* button,
    gpointer   user_data
)
{
    launch_action(GPOINTER_TO_INT(user_data));
}

static gboolean on_focus_out(
    GtkWidget* widget,
    GdkEvent*  event,
    gpointer   user_data
)
{
    gtk_widget_hide(widget);
    return TRUE;
}

static void on_selection_done(
    GtkWidget* widget,
    StartMenu* start_menu
)
{
    gtk_widget_hide(GTK_WIDGET(start_menu));
}
