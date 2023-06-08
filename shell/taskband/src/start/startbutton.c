#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <wintc-comgtk.h>

#include "../meta.h"
#include "startbutton.h"
#include "startmenu.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartButtonPrivate
{
    StartMenu* menu;

    gboolean synchronizing;
};

struct _StartButtonClass
{
    GtkToggleButtonClass __parent__;
};

struct _StartButton
{
    GtkToggleButton __parent__;

    StartButtonPrivate* priv;
};

//
// FORWARD DECLARATIONS
//
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
G_DEFINE_TYPE_WITH_CODE(
    StartButton,
    start_button,
    GTK_TYPE_TOGGLE_BUTTON,
    G_ADD_PRIVATE(StartButton)
)

static void start_button_class_init(
    WINTC_UNUSED(StartButtonClass* klass)
) {}

static void start_button_init(
    StartButton* self
)
{
    self->priv = start_button_get_instance_private(self);

    self->priv->synchronizing = FALSE;

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
    // NOTE: We add two labels for uppercase and lowercase 'start' text, this is so
    //       that themes can hide one or the other via font-size: 0pt
    //
    GtkWidget* start_label_l = gtk_label_new(_("start"));
    GtkWidget* start_label_u = gtk_label_new(_("Start"));

    gtk_box_pack_start(GTK_BOX(outer_box), icon_box,      FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer_box), start_label_l, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(outer_box), start_label_u, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(self), outer_box);

    gtk_widget_set_tooltip_text(GTK_WIDGET(self), _("Click here to begin"));

    // Add style class
    //
    wintc_widget_add_style_class(GTK_WIDGET(self), "xp-start-button");
    wintc_widget_add_style_class(icon_box,         "xp-flag");
    wintc_widget_add_style_class(start_label_l,    "lower");
    wintc_widget_add_style_class(start_label_u,    "upper");

    // Create menu
    //
    self->priv->menu = start_menu_new(GTK_WIDGET(self));

    g_signal_connect(
        G_OBJECT(self),
        "clicked",
        G_CALLBACK(on_clicked),
        NULL
    );

    connect_start_menu_closed_signal(
        self->priv->menu,
        G_CALLBACK(on_start_menu_hidden)
    );
}

//
// PUBLIC FUNCTIONS
//
GtkWidget* start_button_new(void)
{
    return GTK_WIDGET(
        g_object_new(TYPE_START_BUTTON, NULL)
    );
}

//
// CALLBACKS
//
static void on_clicked(
    GtkButton* button,
    WINTC_UNUSED(gpointer user_data)
)
{
    StartButton* start_button = START_BUTTON(button);

    if (start_button->priv->synchronizing)
    {
        return;
    }

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
    {
        start_menu_present(start_button->priv->menu);
    }
    else
    {
        start_menu_close(start_button->priv->menu);
    }
}

static void on_start_menu_hidden(
    WINTC_UNUSED(GtkWidget* widget),
    gpointer user_data
)
{
    StartButton* start_button = START_BUTTON(user_data);

    start_button->priv->synchronizing = TRUE;

    gtk_toggle_button_set_active(
        GTK_TOGGLE_BUTTON(start_button),
        FALSE
    );

    start_button->priv->synchronizing = FALSE;
}
