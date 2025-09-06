#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>
#include <wintc/shelldpa.h>
#include <wintc/shellext.h>
#include <wintc/shlang.h>

#include "../intapi.h"
#include "personal.h"
#include "progmenu.h"
#include "shared.h"
#include "toolbar.h"

//
// FORWARD DECLARATIONS
//
static void wintc_toolbar_start_constructed(
    GObject* object
);
static void wintc_toolbar_start_dispose(
    GObject* object
);

static void on_start_button_toggled(
    GtkToggleButton* self,
    gpointer         user_data
);

//
// GTK TYPE DEFINITION & CTORS
//
G_DEFINE_TYPE(
    WinTCToolbarStart,
    wintc_toolbar_start,
    WINTC_TYPE_SHEXT_UI_CONTROLLER
)

static void wintc_toolbar_start_class_init(
    WinTCToolbarStartClass* klass
)
{
    GObjectClass* object_class = G_OBJECT_CLASS(klass);

    object_class->constructed = wintc_toolbar_start_constructed;
    object_class->dispose     = wintc_toolbar_start_dispose;
}

static void wintc_toolbar_start_init(
    WinTCToolbarStart* self
)
{
    // Default states
    //
    self->sync_button                = FALSE;
    self->sync_menu_should_close     = FALSE;

    // Apply stylesheet for this toolbar
    //
    GtkCssProvider* css = gtk_css_provider_new();

    gtk_css_provider_load_from_resource(
        css,
        "/uk/oddmatics/wintc/taskband/start-menu.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(css),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    // Initialize progmenu
    //
    self->progmenu = wintc_toolbar_start_progmenu_new();
}

//
// CLASS VIRTUAL METHODS
//
static void wintc_toolbar_start_constructed(
    GObject* object
)
{
    (G_OBJECT_CLASS(wintc_toolbar_start_parent_class))->constructed(object);

    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(object);

    // Create Start button as the toolbar
    //
    GtkBuilder* builder =
        gtk_builder_new_from_resource(
            "/uk/oddmatics/wintc/taskband/start-button.ui"
        );

    wintc_lc_builder_preprocess_widget_text(builder);

    wintc_builder_get_objects(
        builder,
        "start-button", &(toolbar_start->start_button),
        NULL
    );

    wintc_ishext_ui_host_get_ext_widget(
        wintc_shext_ui_controller_get_ui_host(
            WINTC_SHEXT_UI_CONTROLLER(object)
        ),
        WINTC_TASKBAND_HOSTEXT_TOOLBAR,
        GTK_TYPE_WIDGET,
        toolbar_start->start_button
    );

    g_object_unref(G_OBJECT(builder));

    // Create Start menu widget
    // FIXME: Hard coded to the personal menu for now, 'til DBus and stuff is
    //        understood and we can support a 'Use classic menu' property
    //
    create_personal_menu(toolbar_start);

    // Attach signals for popup
    //
    g_signal_connect(
        toolbar_start->start_button,
        "toggled",
        G_CALLBACK(on_start_button_toggled),
        toolbar_start
    );
}

static void wintc_toolbar_start_dispose(
    GObject* object
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(object);

    // FIXME: Only worrying about destroying the personal menu for now until
    //        the classic menu is available
    //
    destroy_personal_menu(toolbar_start);

    // Destroy progmenu - ensures the data will be saved
    //
    g_object_unref(toolbar_start->progmenu);

    (G_OBJECT_CLASS(wintc_toolbar_start_parent_class))->dispose(object);
}

//
// PUBLIC FUNCTIONS
//
void wintc_toolbar_start_toggle_menu(
    WinTCToolbarStart* toolbar_start
)
{
    GtkToggleButton* start_button =
        GTK_TOGGLE_BUTTON(toolbar_start->start_button);

    // Rate-limit toggling, prevents glitchy behaviour especially when launched
    // via cmdline/keyboard shortcut (so the menu loses focus then immediately
    // toggles open)
    //
    if (g_get_monotonic_time() - toolbar_start->time_menu_closed < 150000)
    {
        return;
    }

    gtk_toggle_button_set_active(
        start_button,
        !gtk_toggle_button_get_active(start_button)
    );
}

//
// CALLBACKS
//
static void on_start_button_toggled(
    GtkToggleButton* self,
    gpointer         user_data
)
{
    WinTCToolbarStart* toolbar_start = WINTC_TOOLBAR_START(user_data);

    if (toolbar_start->sync_button)
    {
        return;
    }

    // FIXME: Only dealing with personal menu here until class menu exists
    //
    if (gtk_toggle_button_get_active(self))
    {
        open_personal_menu(toolbar_start);
    }
    else
    {
        close_personal_menu(toolbar_start);
    }
}

