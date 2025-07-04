#ifndef __MSGINA_IF_AUTHUI_H__
#define __MSGINA_IF_AUTHUI_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "logon.h"

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_IGINA_AUTH_UI (wintc_igina_auth_ui_get_type())

G_DECLARE_INTERFACE(
    WinTCIGinaAuthUI,
    wintc_igina_auth_ui,
    WINTC,
    IGINA_AUTH_UI,
    GObject
)

struct _WinTCIGinaAuthUIInterface
{
    GTypeInterface base_iface;
};

#endif
