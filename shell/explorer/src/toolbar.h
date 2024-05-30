#ifndef __TOOLBAR_H__
#define __TOOLBAR_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExplorerToolbarClass
{
    GObjectClass __parent__;
} WinTCExplorerToolbarClass;

typedef struct _WinTCExplorerToolbar
{
    GObject __parent__;

    GtkWidget* owner_explorer_wnd;
    GtkWidget* toolbar;
} WinTCExplorerToolbar;

#define WINTC_TYPE_EXPLORER_TOOLBAR            (wintc_explorer_toolbar_get_type())
#define WINTC_EXPLORER_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXPLORER_TOOLBAR, WinTCExplorerToolbar))
#define WINTC_EXPLORER_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXPLORER_TOOLBAR, WinTCExplorerToolbarClass))
#define IS_WINTC_EXPLORER_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXPLORER_TOOLBAR))
#define IS_WINTC_EXPLORER_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXPLORER_TOOLBAR))
#define WINTC_EXPLORER_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXPLORER_TOOLBAR, WinTCExplorerToolbar))

GType wintc_explorer_toolbar_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
GtkWidget* wintc_explorer_toolbar_get_toolbar(
    WinTCExplorerToolbar* toolbar
);

#endif
