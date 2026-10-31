#ifndef __NET_CLIENT_H__
#define __NET_CLIENT_H__

#include <glib.h>
#include <glib-object.h>

//
// The client keeps the list of network adapters that get a tray icon, it is
// implemented once per backend:
//   - nm/  - NetworkManager, through libnm
//   - bsd/ - FreeBSD, through the kernel and wpa_supplicant directly
//
// Only one backend is built, see the taskband's CMakeLists.txt
//

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
