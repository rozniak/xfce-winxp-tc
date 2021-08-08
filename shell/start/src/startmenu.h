#ifndef __STARTMENU_H__
#define __STARTMENU_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _StartMenuPrivate StartMenuPrivate;
typedef struct _StartMenuClass   StartMenuClass;
typedef struct _StartMenu        StartMenu;

#define TYPE_START_MENU            (start_menu_get_type())
#define START_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_START_MENU, StartMenu))
#define START_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_START_MENU, StartMenuClass))
#define IS_START_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_START_MENU))
#define IS_START_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_START_MENU))
#define START_MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_START_MENU, StartMenuClass))

GType start_menu_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
void start_menu_get_popup_position(
    GtkWidget*       menu,
    XfcePanelPlugin* plugin,
    GtkWidget*       widget,
    gint*            x,
    gint*            y
);

G_END_DECLS

#endif
