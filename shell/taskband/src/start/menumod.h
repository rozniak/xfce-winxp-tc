#ifndef __MENUMOD_H__
#define __MENUMOD_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCMenuModdedClass WinTCMenuModdedClass;
typedef struct _WinTCMenuModded      WinTCMenuModded;

#define WINTC_TYPE_MENU_MODDED            (wintc_menu_modded_get_type())
#define WINTC_MENU_MODDED(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_MENU_MODDED, WinTCMenuModded))
#define WINTC_MENU_MODDED_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_MENU_MODDED, WinTCMenuModdedClass))
#define IS_WINTC_MENU_MODDED(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_MENU_MODDED))
#define IS_WINTC_MENU_MODDED_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_MENU_MODDED))
#define WINTC_MENU_MODDED_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_MENU_MODDED, WinTCMenuModded))

GType wintc_menu_modded_get_type(void) G_GNUC_CONST;

#endif
