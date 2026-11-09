#ifndef __NETSHELL_CLIENT_H__
#define __NETSHELL_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

//
// GTK OOP BOILERPLATE
//
#define WINTC_TYPE_NET_CLIENT (wintc_net_client_get_type())

G_DECLARE_FINAL_TYPE(
    WinTCNetClient,
    wintc_net_client,
    WINTC,
    NET_CLIENT,
    GObject
)

//
// PUBLIC FUNCTIONS
//
WinTCNetClient* wintc_net_client_new(void);

GPtrArray* wintc_net_client_get_devices(
    WinTCNetClient* client
);

#endif
