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
typedef struct _SystrayPluginClass SystrayPluginClass;
typedef struct _SystrayPlugin      SystrayPlugin;

#define TYPE_SYSTRAY_PLUGIN            (systray_plugin_get_type())
#define SYSTRAY_PLUGIN(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_SYSTRAY_PLUGIN, SystrayPlugin))
#define SYSTRAY_PLUGIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_SYSTRAY_PLUGIN, SystrayPluginClass))
#define IS_SYSTRAY_PLUGIN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_SYSTRAY_PLUGIN))
#define IS_SYSTRAY_PLUGIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_SYSTRAY_PLUGIN))
#define SYSTRAY_PLUGIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_SYSTRAY_PLUGIN, SystrayPluginClass))

GType systray_plugin_get_type(void) G_GNUC_CONST;

void systray_plugin_register_type(
    XfcePanelTypeModule* type_module
);

G_END_DECLS

#endif
