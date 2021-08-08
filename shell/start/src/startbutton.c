#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>

#include "plugin.h"
#include "startbutton.h"
#include "startmenu.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartButtonClass
{
    GtkToggleButtonClass __parent__;
};

struct _StartButton
{
    GtkToggleButton __parent__;

    GtkWidget*       menu;
    XfcePanelPlugin* plugin;
};

//
// FORWARD DECLARATIONS
//
static void start_button_finalize(
    GObject* object
);

static void on_clicked(
    GtkButton* button,
    gpointer   user_data
);

static void on_start_menu_hidden(
    GtkWidget* widget,
    gpointer   user_data
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(StartButton, start_button, GTK_TYPE_TOGGLE_BUTTON);

static void start_button_class_init(
    StartButtonClass* klass
)
{
    GObjectClass* gclass = G_OBJECT_CLASS(klass);

    gclass->finalize = start_button_finalize;
}

static void start_button_init(
    StartButton* self
)
{
    //
    // The layout here - we use a box to represent the icon next to the 'start' text
    // so that themes can apply their own icon here (important because the Plex style
    // has a white Windows flag instead of the coloured one in Luna and other XP
    // styles)
    //

    // Outer box
    //
    GtkWidget* outer_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // 'Flag icon' box
    //
    GtkWidget* icon_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // Start text
    //
    GtkWidget* start_label = gtk_label_new("start"); // FIXME: Use localized string

    gtk_box_pack_start(GTK_BOX(outer_box), icon_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer_box), start_label, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(self), outer_box);

    // FIXME: Use localized string
    //
    gtk_widget_set_tooltip_text(GTK_WIDGET(self), "Click here to begin");

    // Add style class
    //
    gtk_widget_add_style_class(GTK_WIDGET(self), "xp-start-button");
    gtk_widget_add_style_class(icon_box,         "xp-flag");
}

//
// FINALIZE
//
static void start_button_finalize(
    GObject* object
)
{
    (*G_OBJECT_CLASS(start_button_parent_class)->finalize) (object);
}

//
// PUBLIC FUNCTIONS
//
void start_button_attach_menu(
    StartButton* button,
    StartMenu*   menu
)
{
    if (button->menu != NULL)
    {
        g_warning("Attempt to attach a second menu.");
        return;
    }

    button->menu = GTK_WIDGET(menu);

    g_signal_connect(
        G_OBJECT(menu),
        "hide",
        G_CALLBACK(on_start_menu_hidden),
        button
    );
    g_signal_connect(
        G_OBJECT(button),
        "clicked",
        G_CALLBACK(on_clicked),
        NULL
    );
}

void start_button_attach_plugin(
    StartButton* button,
    StartPlugin* plugin
)
{
    if (button->plugin != NULL)
    {
        g_warning("Attempt to attach a second plugin.");
        return;
    }

    button->plugin = XFCE_PANEL_PLUGIN(plugin);
}

//
// CALLBACKS
//
static void on_clicked(
    GtkButton* button,
    gpointer   user_data
)
{
    StartButton* start_button = START_BUTTON(button);

    if (
        start_button->menu   == NULL ||
        start_button->plugin == NULL
    )
    {
        g_critical("Attachments are incomplete.");
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), FALSE);
        return;
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
    {
        gint x, y;

        gtk_window_present_with_time(
            GTK_WINDOW(start_button->menu),
            GDK_CURRENT_TIME
        );

        start_menu_get_popup_position(
            start_button->menu,
            start_button->plugin,
            GTK_WIDGET(start_button),
            &x,
            &y
        );

        gtk_window_move(GTK_WINDOW(start_button->menu), x, y);
    }
    else
    {
        gtk_widget_hide(start_button->menu);
    }
}

static void on_start_menu_hidden(
    GtkWidget* widget,
    gpointer   user_data
)
{
    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(user_data),
        FALSE
    );
}
