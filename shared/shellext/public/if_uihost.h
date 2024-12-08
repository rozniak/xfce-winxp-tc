/** @file */

#ifndef __SHELLEXT_IF_UIHOST_H__
#define __SHELLEXT_IF_UIHOST_H__

#include <glib.h>
#include <gtk/gtk.h>

//
// GTK INTERFACE DEFINITIONS
//
#define WINTC_TYPE_ISHEXT_UI_HOST (wintc_ishext_ui_host_get_type())

G_DECLARE_INTERFACE(
    WinTCIShextUIHost,
    wintc_ishext_ui_host,
    WINTC,
    ISHEXT_UI_HOST,
    GObject
)

struct _WinTCIShextUIHostInterface
{
    GTypeInterface base_iface;

    GtkWidget* (*get_ext_widget) (
        WinTCIShextUIHost* host,
        guint              ext_id,
        GType              expected_type,
        gpointer           ctx
    );
};

//
// INTERFACE METHODS
//
GtkWidget* wintc_ishext_ui_host_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
);

#endif
