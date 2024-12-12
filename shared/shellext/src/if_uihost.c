#include <glib.h>
#include <wintc/comgtk.h>

#include "../public/if_uihost.h"

//
// GTK INTERFACE DEFINITIONS & CTORS
//
G_DEFINE_INTERFACE(
    WinTCIShextUIHost,
    wintc_ishext_ui_host,
    G_TYPE_OBJECT
)

static void wintc_ishext_ui_host_default_init(
    WINTC_UNUSED(WinTCIShextUIHostInterface* iface)
) {}

//
// INTERFACE METHODS
//
GtkWidget* wintc_ishext_ui_host_get_ext_widget(
    WinTCIShextUIHost* host,
    guint              ext_id,
    GType              expected_type,
    gpointer           ctx
)
{
    WinTCIShextUIHostInterface* iface =
        WINTC_ISHEXT_UI_HOST_GET_IFACE(host);

    return iface->get_ext_widget(host, ext_id, expected_type, ctx);
}
