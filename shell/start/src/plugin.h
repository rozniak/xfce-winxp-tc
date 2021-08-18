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
typedef struct _StartPluginClass StartPluginClass;
typedef struct _StartPlugin      StartPlugin;

#define TYPE_START_PLUGIN            (start_plugin_get_type())
#define START_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_START_PLUGIN, StartPlugin))
#define START_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_START_PLUGIN, StartPluginClass))
#define IS_START_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_START_PLUGIN))
#define IS_START_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_START_PLUGIN))
#define START_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_START_PLUGIN, StartPluginClass))

GType start_plugin_get_type(void) G_GNUC_CONST;

void start_plugin_register_type(XfcePanelTypeModule* type_module);

G_END_DECLS

#endif
