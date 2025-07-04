#include <glib.h>
#include <gtk/gtk.h>
#include <wintc/comgtk.h>

#include "../public/if_authui.h"
#include "../public/logon.h"

//
// GTK TYPE DEFINITIONS & CTORS
//
G_DEFINE_INTERFACE(
    WinTCIGinaAuthUI,
    wintc_igina_auth_ui,
    G_TYPE_OBJECT
)

static void wintc_igina_auth_ui_default_init(
    WinTCIGinaAuthUIInterface* iface
)
{
    g_object_interface_install_property(
        iface,
        g_param_spec_object(
            "logon-session",
            "LogonSession",
            "The logon session host.",
            WINTC_TYPE_GINA_LOGON_SESSION,
            G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY
        )
    );
}
