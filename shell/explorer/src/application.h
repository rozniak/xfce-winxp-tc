#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <glib.h>
#include <gtk/gtk.h>

//
// GTK OOP BOILERPLATE
//
typedef struct _WinTCExplorerApplicationClass WinTCExplorerApplicationClass;
typedef struct _WinTCExplorerApplication      WinTCExplorerApplication;

#define WINTC_TYPE_EXPLORER_APPLICATION            (wintc_explorer_application_get_type())
#define WINTC_EXPLORER_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), WINTC_TYPE_EXPLORER_APPLICATION, WinTCExplorerApplication))
#define WINTC_EXPLORER_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), WINTC_TYPE_EXPLORER_APPLICATION, WinTCExplorerApplicationClass))
#define IS_WINTC_EXPLORER_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), WINTC_TYPE_EXPLORER_APPLICATION))
#define IS_WINTC_EXPLORER_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), WINTC_TYPE_EXPLORER_APPLICATION))
#define WINTC_EXPLORER_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), WINTC_TYPE_EXPLORER_APPLICATION, WinTCExplorerApplicationClass))

GType wintc_explorer_application_get_type(void) G_GNUC_CONST;

//
// PUBLIC FUNCTIONS
//
WinTCExplorerApplication* wintc_explorer_application_new(void);

GdkPixbuf* wintc_explorer_application_get_throbber_pixbuf(void);

#endif
