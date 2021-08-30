#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

#include "clock.h"
#include "plugin.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _SystrayPluginClass
{
    XfcePanelPluginClass __parent__;
};

struct _SystrayPlugin
{
    XfcePanelPlugin __parent__;

    GtkWidget* box;
    GtkWidget* clock;
};

//
// FORWARD DECLARATIONS
//
static void systray_plugin_construct(
    XfcePanelPlugin* plugin
);

//
// GTK TYPE DEFINITION & CTORS
//
XFCE_PANEL_DEFINE_PLUGIN(SystrayPlugin, systray_plugin);

static void systray_plugin_class_init(
    SystrayPluginClass* klass
)
{
    XfcePanelPluginClass* plugin_class;

    plugin_class = XFCE_PANEL_PLUGIN_CLASS(klass);

    plugin_class->construct = systray_plugin_construct;
}

static void systray_plugin_init(
    SystrayPlugin* systray_plugin
)
{
    // nothing
}

static void systray_plugin_construct(
    XfcePanelPlugin* plugin
)
{
    SystrayPlugin* systray = SYSTRAY_PLUGIN(plugin);

    systray->box   = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    systray->clock = GTK_WIDGET(g_object_new(TYPE_TRAY_CLOCK, NULL));

    gtk_container_add(GTK_CONTAINER(plugin),       systray->box);
    gtk_container_add(GTK_CONTAINER(systray->box), systray->clock);

    // Attach systray styles
    //
    GtkStyleContext* tray_styles = gtk_widget_get_style_context(systray->box);

    gtk_style_context_add_class(tray_styles, "xp-systray");

    // Show all now
    //
    gtk_widget_show_all(GTK_WIDGET(systray));
}
