#ifndef __NET_NM_BACKEND_H__
#define __NET_NM_BACKEND_H__

#include <glib.h>
#include <NetworkManager.h>

#include "../device.h"

//
// INTERNAL FUNCTIONS - NetworkManager backend only
//
WinTCNetDevice* wintc_net_device_new(
    NMClient* nm_client,
    NMDevice* nm_device
);

NMDevice* wintc_net_device_get_nm_device(
    WinTCNetDevice* device
);

#endif
