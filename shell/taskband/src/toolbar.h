#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCTaskbandToolbarClass
{
    GObjectClass __parent__;
} WinTCTaskbandToolbarClass;

typedef struct _WinTCTaskbandToolbar
{
    GObject __parent__;

    GtkWidget* widget_root;
} WinTCTaskbandToolbar;

#define WINTC_TYPE_TASKBAND_TOOLBAR            (wintc_taskband_toolbar_get_type())
#define WINTC_TASKBAND_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_TASKBAND_TOOLBAR, WinTCTaskbandToolbar))
#define WINTC_TASKBAND_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_TASKBAND_TOOLBAR, WinTCTaskbandToolbarClass))
#define IS_WINTC_TASKBAND_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_TASKBAND_TOOLBAR))
#define IS_WINTC_TASKBAND_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_TASKBAND_TOOLBAR))
#define WINTC_TASKBAND_TOOLBAR_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_TASKBAND_TOOLBAR, WinTCTaskbandToolbar))

GType wintc_taskband_toolbar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_taskband_toolbar_get_root_widget(
    WinTCTaskbandToolbar* toolbar
);

#endif
