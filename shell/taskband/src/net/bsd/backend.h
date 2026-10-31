#ifndef __NET_BSD_BACKEND_H__
#define __NET_BSD_BACKEND_H__

#include <glib.h>

#include "../device.h"

//
// INTERNAL FUNCTIONS - FreeBSD backend only
//
WinTCNetDevice* wintc_net_device_new(
    const gchar*       iface,
    WinTCNetDeviceKind kind
);

// The client owns the routing socket, it calls this when the kernel reports
// a change to the interface, its addresses or routes
//
void wintc_net_device_refresh(
    WinTCNetDevice* device
);

#endif
