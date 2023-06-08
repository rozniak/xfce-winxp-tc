#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <pwd.h>
#include <unistd.h>
#include <wintc-comgtk.h>

#include "../dispproto.h"
#include "../meta.h"
#include "action.h"
#include "placeslist.h"
#include "programslist.h"
#include "startmenu.h"
#include "util.h"

//
// STRUCTURE DEFINITIONS
//
struct _StartMenu
{
    GtkWidget* menu;

    GtkWidget*        main_box;
    GtkWidget*        start_button;
    GtkStyleProvider* userpic_style_provider;
};

//
// FORWARD DECLARATIONS
//
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
static void create_vertical_userpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
);
static void update_userpic(
    StartMenu* start_menu
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
static void on_userpic_clicked(
    GtkWidget*       userpic,
    GdkEventButton*  event
);

//
// PUBLIC FUNCTIONS
//
void connect_start_menu_closed_signal(
    StartMenu* start_menu,
    GCallback  callback
)
{
    g_signal_connect(
        start_menu->menu,
        "hide",
        callback,
        start_menu->start_button
    );
}

void start_menu_close(
    StartMenu* start_menu
)
{
    gtk_widget_hide(start_menu->menu);
}

StartMenu* start_menu_new(
    GtkWidget* start_button
)
{
    StartMenu* start_menu = g_new(StartMenu, 1);

    // On X11, we must use a GtkWindow, on Wayland we use a GtkPopover - the
    // reason for this is because popovers cannot exist outside the bounds of
    // the parent window on X11
    //
    // We use a GtkWindow instead on X11 to work around that, using
    // gtk_window_move() -- which obviously doesn't work on Wayland
    //
    // So... GtkWindow on X11, GtkPopover on Wayland :)
    //
    if (get_display_protocol_in_use() == DISPPROTO_X11)
    {
        start_menu->menu = gtk_window_new(GTK_WINDOW_TOPLEVEL);

        // Set up hints
        //
        gtk_window_set_type_hint(
            GTK_WINDOW(start_menu->menu),
            GDK_WINDOW_TYPE_HINT_POPUP_MENU
        );
        gtk_window_set_resizable(
            GTK_WINDOW(start_menu->menu),
            FALSE
        );
        gtk_window_set_keep_above(
            GTK_WINDOW(start_menu->menu),
            TRUE
        );
        gtk_window_stick(GTK_WINDOW(start_menu->menu));
        gtk_window_set_skip_taskbar_hint(
            GTK_WINDOW(start_menu->menu),
            TRUE
        );
        gtk_window_set_title(
            GTK_WINDOW(start_menu->menu),
            "Start menu"
        );
        gtk_widget_set_events(
            GTK_WIDGET(start_menu->menu),
            GDK_FOCUS_CHANGE_MASK
        );

        // Set up signals
        //
        g_signal_connect(
            start_menu->menu,
            "delete-event",
            G_CALLBACK(gtk_widget_hide_on_delete),
            NULL
        );
        g_signal_connect(
            start_menu->menu,
            "focus-out-event",
            G_CALLBACK(on_focus_out),
            NULL
        );

        // Set titlebar to the fake titlebar box
        //
        GtkWidget* fake_titlebar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        gtk_window_set_titlebar(
            GTK_WINDOW(start_menu->menu),
            fake_titlebar
        );
    }
    else if (get_display_protocol_in_use() == DISPPROTO_WAYLAND)
    {
        start_menu->menu = gtk_popover_new(start_button);
    }

    // Set up structure
    //
    start_menu->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    create_userpane_structure(start_menu, GTK_BOX(start_menu->main_box));
    create_taskcolumns_structure(start_menu, GTK_BOX(start_menu->main_box));
    create_logoffpane_structure(start_menu, GTK_BOX(start_menu->main_box));

    gtk_container_add(GTK_CONTAINER(start_menu->menu), start_menu->main_box);

    // Add style class
    //
    wintc_widget_add_style_class(GTK_WIDGET(start_menu->menu), "xp-start-menu");

    gtk_widget_show_all(start_menu->main_box);

    // Attach button
    //
    start_menu->start_button = start_button;

    return start_menu;
}

void start_menu_present(
    StartMenu* start_menu
)
{
    gint height;
    gint x;
    gint y;

    if (get_display_protocol_in_use() == DISPPROTO_X11)
    {
        gtk_window_present_with_time(
            GTK_WINDOW(start_menu->menu),
            GDK_CURRENT_TIME
        );

        height = gtk_widget_get_allocated_height(start_menu->main_box);

        gdk_window_get_origin(
            gtk_widget_get_window(start_menu->start_button),
            &x,
            &y
        );

        gtk_window_move(
            GTK_WINDOW(start_menu->menu),
            x,
            y - height
        );
    }
    else
    {
        gtk_widget_show(start_menu->menu);
    }
}

//
// PRIVATE FUNCTIONS
//
static void create_logoffpane_structure(
    WINTC_UNUSED(StartMenu* start_menu),
    GtkBox* box
)
{
    GtkWidget* logoffpane_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    //
    // These are a couple ordinary buttons, except we have an extra box before the
    // label so that themes can add an icon
    //
    // The reason for not using the gtk-icon-theme here is because the themes need
    // to be able to override the image, AND they need to provide a 'hot' graphic for
    // when the buttons are hovered
    //

    // Log off button
    //
    GtkWidget* logoff_button   = gtk_button_new();
    GtkWidget* logoff_box      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* logoff_icon_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* logoff_label    = gtk_label_new(_("Log Off"));

    gtk_box_pack_start(GTK_BOX(logoff_box), logoff_icon_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(logoff_box), logoff_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(logoff_button), logoff_box);

    g_signal_connect(
        G_OBJECT(logoff_button),
        "clicked",
        G_CALLBACK(on_action_button_clicked),
        GINT_TO_POINTER(WINTC_ACTION_LOGOFF)
    );

    gtk_widget_set_tooltip_text(
        logoff_button,
        _("Provides options for closing your programs and logging off, or for leaving your programs running and switching to another user.")
    );

    wintc_widget_add_style_class(logoff_icon_box, "logoff-icon");

    // Shut down button
    //
    GtkWidget* shutdown_button   = gtk_button_new();
    GtkWidget* shutdown_box      = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* shutdown_icon_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* shutdown_label    = gtk_label_new(_("Turn Off Computer"));

    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_icon_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(shutdown_box), shutdown_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(shutdown_button), shutdown_box);

    g_signal_connect(
        G_OBJECT(shutdown_button),
        "clicked",
        G_CALLBACK(on_action_button_clicked),
        GINT_TO_POINTER(WINTC_ACTION_SHUTDOWN)
    );

    gtk_widget_set_tooltip_text(
        shutdown_button,
        _("Provides options for turning off or restarting your computer, or for activating Stand By or Hibernate modes.")
    );

    wintc_widget_add_style_class(shutdown_icon_box, "shutdown-icon");

    // Pack box
    //
    gtk_box_pack_end(GTK_BOX(logoffpane_box), shutdown_button, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(logoffpane_box), logoff_button, FALSE, FALSE, 0);
    gtk_box_pack_start(box, logoffpane_box, FALSE, FALSE, 0);

    // Add style class
    //
    wintc_widget_add_style_class(GTK_WIDGET(logoffpane_box), "xp-start-logoffpane");
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

    wintc_signal_connect_list(
        menu_children,
        "enter-notify-event",
        G_CALLBACK(wintc_menu_shell_select_on_enter),
        places_list
    );
    wintc_signal_connect_list(
        menu_children,
        "leave-notify-event",
        G_CALLBACK(wintc_menu_shell_deselect_on_leave),
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
    wintc_widget_add_style_class(GTK_WIDGET(places_box), "xp-start-places-column");
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

    wintc_signal_connect_list(
        menu_children,
        "enter-notify-event",
        G_CALLBACK(wintc_menu_shell_select_on_enter),
        programs_list
    );
    wintc_signal_connect_list(
        menu_children,
        "leave-notify-event",
        G_CALLBACK(wintc_menu_shell_deselect_on_leave),
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
    wintc_widget_add_style_class(GTK_WIDGET(programs_box), "xp-start-programs-column");
}

static void create_taskcolumns_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    // Create structure
    //
    GtkWidget* columns_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    create_vertical_userpane_structure(start_menu, GTK_BOX(columns_box));
    create_programs_structure(start_menu, GTK_BOX(columns_box));
    create_places_structure(start_menu, GTK_BOX(columns_box));

    gtk_box_pack_start(box, columns_box, TRUE, TRUE, 0);

    // Add style class
    //
    wintc_widget_add_style_class(GTK_WIDGET(columns_box), "xp-start-columns");
}

static void create_userpane_structure(
    StartMenu* start_menu,
    GtkBox*    box
)
{
    GtkWidget* userpane_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    
    // User profile picture
    //
    GtkStyleContext* context;
    GtkWidget*       pic_box       = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget*       pic_event_box = gtk_event_box_new();

    start_menu->userpic_style_provider =
        GTK_STYLE_PROVIDER(gtk_css_provider_new());

    context = gtk_widget_get_style_context(pic_box);

    gtk_style_context_add_provider(
        context,
        start_menu->userpic_style_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    update_userpic(start_menu);

    gtk_widget_set_events(pic_event_box, GDK_BUTTON_PRESS_MASK);
    gtk_box_pack_start(GTK_BOX(pic_box), pic_event_box, TRUE, TRUE, 0);

    g_signal_connect(
        pic_event_box,
        "button-press-event",
        G_CALLBACK(on_userpic_clicked),
        NULL
    );

    // Username display
    //
    struct passwd* user_pwd = getpwuid(getuid());

    GtkWidget* username_label = gtk_label_new(user_pwd->pw_name);

    // Construct box
    //
    gtk_box_pack_start(GTK_BOX(userpane_box), pic_box,        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(userpane_box), username_label, FALSE, FALSE, 0);

    gtk_box_pack_start(box, userpane_box, FALSE, FALSE, 0);

    // Add style class
    //
    wintc_widget_add_style_class(userpane_box, "xp-start-userpane");
}

static void create_vertical_userpane_structure(
    WINTC_UNUSED(StartMenu* start_menu),
    GtkBox* box
)
{
    GtkWidget* userpane_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    // Username display
    //
    struct passwd* user_pwd = getpwuid(getuid());

    GtkWidget* username_label = gtk_label_new(user_pwd->pw_name);

    gtk_label_set_angle(GTK_LABEL(username_label), 90);

    // Construct box
    //
    gtk_box_pack_end(GTK_BOX(userpane_box), username_label, FALSE, FALSE, 0);

    gtk_box_pack_start(box, userpane_box, FALSE, FALSE, 0);

    // Add style class
    //
    wintc_widget_add_style_class(userpane_box, "xp-start-vuserpane");
}

static void update_userpic(
    StartMenu* start_menu
)
{
    static gchar* css = NULL;

    if (css == NULL)
    {
        // FIXME: This should read from whatever the XDG path is, probably needs a
        //        g_strdup_printf for the username
        //
        css = "* { background-image: url('/usr/share/wintc/shell-res/fpo-userpic.png'); }";
    }

    // Give GTK a bump that we want to update the pic
    //
    gtk_css_provider_load_from_data(
        GTK_CSS_PROVIDER(start_menu->userpic_style_provider),
        css,
        -1,
        NULL
    );
}

//
// CALLBACKS
//
static void on_action_button_clicked(
    WINTC_UNUSED(GtkButton* button),
    gpointer user_data
)
{
    launch_action(GPOINTER_TO_INT(user_data));
}

static gboolean on_focus_out(
    GtkWidget* widget,
    WINTC_UNUSED(GdkEvent* event),
    WINTC_UNUSED(gpointer  user_data)
)
{
    gtk_widget_hide(widget);
    return TRUE;
}

static void on_selection_done(
    WINTC_UNUSED(GtkWidget* widget),
    StartMenu* start_menu
)
{
    gtk_widget_hide(GTK_WIDGET(start_menu->menu));
}

static void on_userpic_clicked(
    WINTC_UNUSED(GtkWidget* userpic),
    GdkEventButton* event
)
{
    //
    // FIXME: Implement this when the user pic cpl is done!
    //
    if (event->button > 1)
    {
        return;
    }

    GError* error = NULL;

    g_set_error(
        &error,
        WINTC_GENERAL_ERROR,
        WINTC_GENERAL_ERROR_NOTIMPL,
        "Cannot edit user pic yet!"
    );

    wintc_nice_error_and_clear(&error);
}
