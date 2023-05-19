#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _TaskButtonsPluginClass   TaskButtonsPluginClass;
typedef struct _TaskButtonsPlugin        TaskButtonsPlugin;

#define TYPE_TASKBUTTONS_PLUGIN            (taskbuttons_plugin_get_type())
#define TASKBUTTONS_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_TASKBUTTONS_PLUGIN, TaskButtonsPlugin))
#define TASKBUTTONS_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_TASKBUTTONS_PLUGIN, TaskButtonsPluginClass))
#define IS_TASKBUTTONS_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE((obj), TYPE_TASKBUTTONS_PLUGIN))
#define IS_TASKBUTTONS_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_TASKBUTTONS_PLUGIN))
#define TASKBUTTONS_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_TASKBUTTONS_PLUGIN, TaskButtonsPluginClass))

GType taskbuttons_plugin_get_type(void) G_GNUC_CONST;

void taskbuttons_plugin_register_type(
    XfcePanelTypeModule* type_module
);

G_END_DECLS

#endif
