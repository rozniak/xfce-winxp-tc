#include <glib.h>
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>
#include <wintc-comgtk.h>

#include "plugin.h"
#include "taskbuttonbar.h"
#include "windowmonitor.h"

//
// GTK OOP CLASS/INSTANCE DEFINITIONS
//
struct _TaskButtonsPluginClass
{
    XfcePanelPluginClass __parent__;
};

struct _TaskButtonsPlugin
{
    XfcePanelPlugin __parent__;

    TaskButtonBar* button_bar;
    WindowMonitor* window_monitor;
};

//
// FORWARD DECLARATIONS
//
static void taskbuttons_plugin_construct(
    XfcePanelPlugin* plugin
);

//
// GTK TYPE DEFINITION & CTORS
//
XFCE_PANEL_DEFINE_PLUGIN(TaskButtonsPlugin, taskbuttons_plugin)

static void taskbuttons_plugin_class_init(
    TaskButtonsPluginClass* klass
)
{
    XfcePanelPluginClass* plugin_class;

    plugin_class = XFCE_PANEL_PLUGIN_CLASS(klass);

    plugin_class->construct = taskbuttons_plugin_construct;
}

static void taskbuttons_plugin_init(
    TaskButtonsPlugin* self
)
{
    self->button_bar     = TASKBUTTON_BAR(taskbutton_bar_new());
    self->window_monitor = window_monitor_init_management(
                               GTK_CONTAINER(self->button_bar)
                           );

    gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->button_bar));

    wintc_widget_add_style_class(GTK_WIDGET(self), "wintc-taskbuttons");
}

static void taskbuttons_plugin_construct(
    XfcePanelPlugin* plugin
)
{
    gtk_widget_show_all(GTK_WIDGET(plugin));
    xfce_panel_plugin_set_expand(plugin, TRUE);
}
