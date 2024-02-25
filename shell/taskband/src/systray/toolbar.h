#ifndef __TOOLBAR_NOTIF_AREA_H__
#define __TOOLBAR_NOTIF_AREA_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCToolbarNotifAreaClass WinTCToolbarNotifAreaClass;
typedef struct _WinTCToolbarNotifArea      WinTCToolbarNotifArea;

#define TYPE_WINTC_TOOLBAR_NOTIF_AREA            (wintc_toolbar_notif_area_get_type())
#define WINTC_TOOLBAR_NOTIF_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_WINTC_TOOLBAR_NOTIF_AREA, WinTCToolbarNotifArea))
#define WINTC_TOOLBAR_NOTIF_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_WINTC_TOOLBAR_NOTIF_AREA, WinTCToolbarNotifAreaClass))
#define IS_WINTC_TOOLBAR_NOTIF_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_WINTC_TOOLBAR_NOTIF_AREA))
#define IS_WINTC_TOOLBAR_NOTIF_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_WINTC_TOOLBAR_NOTIF_AREA))
#define WINTC_TOOLBAR_NOTIF_AREA_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_WINTC_TOOLBAR_NOTIF_AREA, WinTCToolbarNotifArea))

GType wintc_toolbar_notif_area_get_type(void) G_GNUC_CONST;

#endif

