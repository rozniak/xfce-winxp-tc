#ifndef __STARTMENUITEM_H__
#define __STARTMENUITEM_H__

#include <garcon/garcon.h>
#include <gio/gdesktopappinfo.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <wintc-exec.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _StartMenuItemPrivate StartMenuItemPrivate;
typedef struct _StartMenuItemClass   StartMenuItemClass;
typedef struct _StartMenuItem        StartMenuItem;

#define TYPE_START_MENU_ITEM            (start_menu_item_get_type())
#define START_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_START_MENU_ITEM, StartMenuItem))
#define START_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_START_MENU_ITEM, StartMenuItemClass))
#define IS_START_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_START_MENU_ITEM))
#define IS_START_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_START_MENU_ITEM))
#define START_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_START_MENU_ITEM, StartMenuItemClass))

GType start_menu_item_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* start_menu_item_new_from_action(
    WinTCAction action
);
GtkWidget* start_menu_item_new_from_desktop_entry(
    GDesktopAppInfo* entry,
    const gchar*     generic_name,
    const gchar*     comment
);
GtkWidget* start_menu_item_new_from_garcon_item(
    GarconMenuItem* item,
    const gchar*    generic_name,
    const gchar*    comment
);
void start_menu_item_set_icon_size(
    StartMenuItem* item,
    gint           size
);

G_END_DECLS

#endif
