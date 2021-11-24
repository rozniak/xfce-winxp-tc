#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

#include "plugin.h"
#include "startbutton.h"
#include "startmenu.h"
#include "util.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _StartPluginClass
{
    XfcePanelPluginClass __parent__;
};

struct _StartPlugin
{
    XfcePanelPlugin __parent__;

    GtkWidget* start_button;
    GtkWidget* start_menu;
};

//
// FORWARD DECLARATIONS
//
static void start_plugin_construct(
    XfcePanelPlugin* plugin
);
static void start_plugin_free_data(
    XfcePanelPlugin* plugin
);

//
// GTK TYPE DEFINITION & CTORS
//
XFCE_PANEL_DEFINE_PLUGIN(StartPlugin, start_plugin)

static void start_plugin_class_init(
    StartPluginClass* klass
)
{
    XfcePanelPluginClass* plugin_class;

    plugin_class = XFCE_PANEL_PLUGIN_CLASS(klass);

    plugin_class->construct = start_plugin_construct;
    plugin_class->free_data = start_plugin_free_data;
}

static void start_plugin_init(
    StartPlugin* start_plugin
)
{
    // nothing
}

static void start_plugin_construct(
    XfcePanelPlugin* plugin
)
{
    StartPlugin*    start  = START_PLUGIN(plugin);
    GtkCssProvider* styles = gtk_css_provider_new();

    // Load global stylesheet
    //
    gtk_css_provider_load_from_resource(
        styles,
        "/uk/oddmatics/wintc/xp-start/appstyles.css"
    );

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(styles),
        GTK_STYLE_PROVIDER_PRIORITY_FALLBACK
    );

    // Set up plugin widgets
    //
    start->start_button = GTK_WIDGET(g_object_new(TYPE_START_BUTTON, NULL));
    start->start_menu   = GTK_WIDGET(g_object_new(TYPE_START_MENU,   NULL));

    start_button_attach_menu(
        START_BUTTON(start->start_button),
        START_MENU(start->start_menu)
    );
    start_button_attach_plugin(
        START_BUTTON(start->start_button),
        start
    );

    gtk_widget_add_style_class(GTK_WIDGET(plugin), "xp-start-plugin");

    gtk_container_add(GTK_CONTAINER(plugin), start->start_button);
    gtk_widget_show_all(start->start_button);
}

//
// FINALIZE
//
static void start_plugin_free_data(
    XfcePanelPlugin* plugin
)
{
    StartPlugin* start = START_PLUGIN(plugin);

    gtk_widget_destroy(start->start_menu);
}
